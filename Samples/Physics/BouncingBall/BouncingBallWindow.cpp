// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "BouncingBallWindow.h"

int main(int, char const*[])
{
#if defined(_DEBUG)
    LogReporter reporter(
        "LogReport.txt",
        Logger::Listener::LISTEN_FOR_ALL,
        Logger::Listener::LISTEN_FOR_ALL,
        Logger::Listener::LISTEN_FOR_ALL,
        Logger::Listener::LISTEN_FOR_ALL);
#endif

    Window::Parameters parameters(L"BouncingBallWindow", 0, 0, 512, 512);
    auto window = TheWindowSystem.Create<BouncingBallWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
    TheWindowSystem.Destroy<BouncingBallWindow>(window);
    return 0;
}

BouncingBallWindow::BouncingBallWindow(Parameters& parameters)
    :
    Window3(parameters),
    mSimTime(0.0f),
#if defined(BOUNCING_BALL_SINGLE_STEP)
    mSimDelta(0.05f)
#else
    mSimDelta(0.0005f)
#endif
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    mEngine->SetClearColor({ 0.5f, 0.0f, 1.0f, 1.0f });
    mWireState = std::make_shared<RasterizerState>();
    mWireState->fillMode = RasterizerState::FILL_WIREFRAME;

    CreateScene();

    // Initial update of objects.
    mScene->Update();
    mBallNode->Update();

    // Initialize ball with correct transformations.
    PhysicsTick();

    float angle = static_cast<float>(0.1 * GTE_C_PI);
    float cs = cos(angle), sn = sin(angle);
    InitializeCamera(60.0f, GetAspectRatio(), 1.0f, 1000.0f, 0.1f, 0.01f,
        { 6.75f, 0.0f, 2.3f }, { -cs, 0.0f, -sn }, { -sn, 0.0f, cs });
    mPVWMatrices.Update();
}

void BouncingBallWindow::OnIdle()
{
#if !defined(BOUNCING_BALL_SINGLE_STEP)
    PhysicsTick();
#endif
    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }
    GraphicsTick();
}

bool BouncingBallWindow::OnCharPress(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'w':  // toggle wireframe
    case 'W':
        if (mEngine->GetRasterizerState() == mWireState)
        {
            mEngine->SetDefaultRasterizerState();
        }
        else
        {
            mEngine->SetRasterizerState(mWireState);
        }
        return true;

    case 's':  // toggle scaling
        mBall->DoAffine() = !mBall->DoAffine();
        return true;

#if defined(BOUNCING_BALL_SINGLE_STEP)
    case 'g':
        mSimTime += mSimDelta;
        PhysicsTick();
        return true;
#endif
    }

    return Window3::OnCharPress(key, x, y);
}

bool BouncingBallWindow::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");
    std::vector<std::string> inputs =
    {
        "BallTexture.png",
        "Floor.png",
        "Wall1.png"
    };

    for (auto const& input : inputs)
    {
        if (mEnvironment.GetPath(input) == "")
        {
            LogError("Cannot find file " + input);
            return false;
        }
    }

    return true;
}

void BouncingBallWindow::CreateScene()
{
    mScene = std::make_shared<Node>();

    CreateBall();
    CreateFloor();
    CreateWall();
    mScene->AttachChild(mFloor);
    mScene->AttachChild(mWall);

    std::vector<std::shared_ptr<Visual>> planes = { mFloor };
    std::vector<float> reflectances = { 0.2f };
    mPlanarReflectionEffect = std::make_unique<PlanarReflectionEffect>(planes, reflectances);

    mSceneVisibleSet.push_back(mWall.get());
    mBallNodeVisibleSet.push_back(mBall->GetMesh().get());
}

void BouncingBallWindow::CreateBall()
{
    std::string path = mEnvironment.GetPath("BallTexture.png");
    std::shared_ptr<Texture2> texture = WICFileIO::Load(path, false);
    std::shared_ptr<Texture2Effect> effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::MIN_L_MAG_L_MIP_P, SamplerState::WRAP, SamplerState::WRAP);

    mBall = std::make_unique<DeformableBall>(1.0f, 2.0f, effect);
    mBallNode = std::make_shared<Node>();
    std::shared_ptr<Visual> mesh = mBall->GetMesh();
    mBallNode->AttachChild(mesh);
    mPVWMatrices.Subscribe(mesh->worldTransform, effect->GetPVWMatrixConstant());
}

void BouncingBallWindow::CreateFloor()
{
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);

    std::shared_ptr<VertexBuffer> vbuffer = std::make_shared<VertexBuffer>(vformat, 4);
    Vertex* vertex = vbuffer->Get<Vertex>();
    float const xExtent = 8.0f, yExtent = 16.0f, zValue = 0.0f;
    vertex[0].position = { -xExtent, -yExtent, zValue };
    vertex[1].position = { +xExtent, -yExtent, zValue };
    vertex[2].position = { +xExtent, +yExtent, zValue };
    vertex[3].position = { -xExtent, +yExtent, zValue };
    vertex[0].tcoord = { 0.0f, 0.0f };
    vertex[1].tcoord = { 1.0f, 0.0f };
    vertex[2].tcoord = { 1.0f, 1.0f };
    vertex[3].tcoord = { 0.0f, 1.0f };

    std::shared_ptr<IndexBuffer> ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, 2, sizeof(unsigned int));
    unsigned int* indices = ibuffer->Get<unsigned int>();
    indices[0] = 0;  indices[1] = 1;  indices[2] = 2;
    indices[3] = 0;  indices[4] = 2;  indices[5] = 3;

    std::string path = mEnvironment.GetPath("Floor.png");
    std::shared_ptr<Texture2> texture = WICFileIO::Load(path, false);
    std::shared_ptr<Texture2Effect> effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::MIN_L_MAG_L_MIP_P, SamplerState::WRAP, SamplerState::WRAP);

    mFloor = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mFloor->worldTransform, effect->GetPVWMatrixConstant());
}

void BouncingBallWindow::CreateWall()
{
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);

    std::shared_ptr<VertexBuffer> vbuffer = std::make_shared<VertexBuffer>(vformat, 4);
    Vertex* vertex = vbuffer->Get<Vertex>();
    float const xValue = -8.0f, yExtent = 16.0f, zExtent = 16.0f, maxTCoord = 4.0f;

    vertex[0].position = { xValue, -yExtent, 0.0f };
    vertex[1].position = { xValue, +yExtent, 0.0f };
    vertex[2].position = { xValue, +yExtent, zExtent };
    vertex[3].position = { xValue, -yExtent, zExtent };
    vertex[0].tcoord = { 0.0f, 0.0f };
    vertex[1].tcoord = { maxTCoord, 0.0f };
    vertex[2].tcoord = { maxTCoord, maxTCoord };
    vertex[3].tcoord = { 0.0f, maxTCoord };

    std::shared_ptr<IndexBuffer> ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, 2, sizeof(unsigned int));
    unsigned int* indices = ibuffer->Get<unsigned int>();
    indices[0] = 0;  indices[1] = 1;  indices[2] = 2;
    indices[3] = 0;  indices[4] = 2;  indices[5] = 3;

    std::string path = mEnvironment.GetPath("Wall1.png");
    std::shared_ptr<Texture2> texture = WICFileIO::Load(path, false);
    std::shared_ptr<Texture2Effect> effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::MIN_L_MAG_L_MIP_P, SamplerState::WRAP, SamplerState::WRAP);

    mWall = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mWall->worldTransform, effect->GetPVWMatrixConstant());
}

void BouncingBallWindow::PhysicsTick()
{
    // Update the ball.
    mBall->DoSimulationStep(mSimTime);
    mEngine->Update(mBall->GetMesh()->GetVertexBuffer());

    // Get the ball parameters.
    float period = mBall->GetPeriod();
    float tMin = mBall->GetMinActive();
    float tMax = mBall->GetMaxActive();

    // Translate the ball.
    float const yMax = 2.5f, zMax = 0.75f;
    float yTrn, zTrn, ratio, amp;
    float time = fmod(mSimTime, 2.0f * period);
    if (time < tMin)
    {
        ratio = time / tMin;
        yTrn = yMax * ratio;
        zTrn = zMax * (1.0f - ratio*ratio);
    }
    else if (time < tMax)
    {
        yTrn = yMax;
        amp = mBall->GetAmplitude(time);
        if (amp <= 0.999f)
        {
            zTrn = -(1.0f - sqrt(1.0f - amp + amp * amp)) / (1.0f - amp);
        }
        else
        {
            zTrn = -0.5f;
        }
    }
    else if (time < period + tMin)
    {
        yTrn = -yMax * (time - period) / tMin;
        zTrn = zMax * (time - tMax) * (period + tMin - time) / (tMin * (period - tMax));
    }
    else if (time < period + tMax)
    {
        yTrn = -yMax;
        amp = mBall->GetAmplitude(time - period);
        if (amp <= 0.999f)
        {
            zTrn = -(1.0f - sqrt(1.0f - amp + amp*amp)) / (1.0f - amp);
        }
        else
        {
            zTrn = -0.5f;
        }
    }
    else
    {
        yTrn = yMax * (time - 2.0f * period) / (period - tMax);
        zTrn = zMax * (time - (period + tMax)) * (2.0f * period + tMin - time) / (tMin * (period - tMax));
    }
    mBallNode->localTransform.SetTranslation(0.0f, yTrn, zTrn);

    // Rotate the ball.
    float angle = (1.0f + yTrn) * (float)GTE_C_HALF_PI / yMax;
    mBallNode->localTransform.SetRotation(AxisAngle<4, float>(Vector4<float>::Unit(2), angle));

    // Update the scene graph.
    mBallNode->Update();
    mPVWMatrices.Update();

    // Next simulation time.
    mSimTime += mSimDelta;
}

void BouncingBallWindow::GraphicsTick()
{
    mTimer.Measure();

    mEngine->ClearBuffers();
    mEngine->Draw(mSceneVisibleSet);
    mPlanarReflectionEffect->Draw(mEngine, mBallNodeVisibleSet, mPVWMatrices);

    std::array<float, 4> textColor{ 0.0f, 0.0f, 0.0f, 1.0f };
    mEngine->Draw(8, mYSize - 8, textColor, mTimer.GetFPS());
    mEngine->Draw(128, mYSize - 8, textColor, "time = " + std::to_string(mSimTime));

    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}
