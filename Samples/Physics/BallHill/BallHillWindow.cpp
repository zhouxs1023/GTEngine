// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "BallHillWindow.h"

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

    Window::Parameters parameters(L"BallHillWindow", 0, 0, 640, 480);
    auto window = TheWindowSystem.Create<BallHillWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
    TheWindowSystem.Destroy<BallHillWindow>(window);
    return 0;
}

BallHillWindow::BallHillWindow(Parameters& parameters)
    :
    Window3(parameters)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    mEngine->SetClearColor({ 0.839215f, 0.894117f, 0.972549f, 1.0f });

    CreateScene();

    float angle = static_cast<float>(0.1 * GTE_C_PI);
    float cs = cos(angle);
    float sn = sin(angle);
    InitializeCamera(60.0f, GetAspectRatio(), 1.0f, 100.0f, 0.001f, 0.001f,
        { 4.0f, 0.0f, 2.0f }, { -cs, 0.0f, -sn }, { -sn, 0.0f, cs });
    mPVWMatrices.Update();
}

void BallHillWindow::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

#if !defined(BALL_HILL_SINGLE_STEP)
    PhysicsTick();
#endif
    GraphicsTick();

    mTimer.UpdateFrameCount();
}

bool BallHillWindow::OnCharPress(unsigned char key, int x, int y)
{
#if defined(BALL_HILL_SINGLE_STEP)
    if (key == 'g' || key == 'G')
    {
        PhysicsTick();
        return true;
    }
#endif
    return Window3::OnCharPress(key, x, y);
}

bool BallHillWindow::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");
    std::vector<std::string> inputs =
    {
        "Grass.png",
        "Gravel.png",
        "BallTexture.png"
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

void BallHillWindow::InitializeModule()
{
    mModule.gravity = 1.0f;
    mModule.a1 = 2.0f;
    mModule.a2 = 1.0f;
    mModule.a3 = 1.0f;
    mModule.radius = 0.1f;

    float time = 0.0f;
    float deltaTime = 0.01f;
    float y1 = 0.0f;
    float y2 = 0.0f;
    float y1Dot = 0.1f;
    float y2Dot = 0.1f;
    mModule.Initialize(time, deltaTime, y1, y2, y1Dot, y2Dot);
}

void BallHillWindow::CreateScene()
{
    InitializeModule();

    mVFormat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    mVFormat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);
    mMeshFactory.SetVertexFormat(mVFormat);

    CreateGround();
    CreateHill();
    CreateBall();
    CreatePath();
    mTrackball.Update();
}

void BallHillWindow::CreateGround()
{
    // Create the ground.  Change the texture repeat pattern.
    mGround = mMeshFactory.CreateRectangle(2, 2, 32.0f, 32.0f);
    VertexBuffer* vbuffer = mGround->GetVertexBuffer().get();
    unsigned int const numVertices = vbuffer->GetNumElements();
    Vertex* vertex = vbuffer->Get<Vertex>();
    for (unsigned int i = 0; i < numVertices; ++i)
    {
        vertex[i].tcoord *= 8.0f;
    }

    // Create a texture effect for the ground.
    std::string path = mEnvironment.GetPath("Grass.png");
    std::shared_ptr<Texture2> texture = WICFileIO::Load(path, true);
    texture->AutogenerateMipmaps();
    std::shared_ptr<Texture2Effect> effect = std::make_shared<Texture2Effect>(mProgramFactory,
        texture, SamplerState::MIN_L_MAG_L_MIP_P, SamplerState::WRAP, SamplerState::WRAP);
    mGround->SetEffect(effect);

    mPVWMatrices.Subscribe(mGround->worldTransform, effect->GetPVWMatrixConstant());
    mTrackball.Attach(mGround);
}

void BallHillWindow::CreateHill()
{
    // Create the hill.  Adjust the disk vertices to form an elliptical paraboloid
    // for the hill.  Change the texture repeat pattern.
    mHill = mMeshFactory.CreateDisk(32, 32, 2.0f);
    VertexBuffer* vbuffer = mHill->GetVertexBuffer().get();
    unsigned int const numVertices = vbuffer->GetNumElements();
    Vertex* vertex = vbuffer->Get<Vertex>();
    for (unsigned int i = 0; i < numVertices; ++i)
    {
        vertex[i].position[2] = mModule.GetHeight(vertex[i].position[0], vertex[i].position[1]);
        vertex[i].tcoord *= 8.0f;
    }

    // Create a texture effect for the hill.
    std::string path = mEnvironment.GetPath("Gravel.png");
    std::shared_ptr<Texture2> texture = WICFileIO::Load(path, true);
    texture->AutogenerateMipmaps();
    std::shared_ptr<Texture2Effect> effect = std::make_shared<Texture2Effect>(mProgramFactory,
        texture, SamplerState::MIN_L_MAG_L_MIP_P, SamplerState::WRAP, SamplerState::WRAP);
    mHill->SetEffect(effect);

    mPVWMatrices.Subscribe(mHill->worldTransform, effect->GetPVWMatrixConstant());
    mTrackball.Attach(mHill);
}

void BallHillWindow::CreateBall()
{
    // Create the ball.
    mBall = mMeshFactory.CreateSphere(16, 16, mModule.radius);

    // Move the ball to the top of the hill.
    Vector3<float> trn = mBall->localTransform.GetTranslation();
    trn[2] = mModule.a3 + mModule.radius;
    mBall->localTransform.SetTranslation(trn);
    UpdateBall();

    // Create a texture effect for the ball.
    std::string path = mEnvironment.GetPath("BallTexture.png");
    std::shared_ptr<Texture2> texture = WICFileIO::Load(path, true);
    texture->AutogenerateMipmaps();
    std::shared_ptr<Texture2Effect> effect = std::make_shared<Texture2Effect>(mProgramFactory,
        texture, SamplerState::MIN_L_MAG_L_MIP_P, SamplerState::WRAP, SamplerState::WRAP);
    mBall->SetEffect(effect);

    mPVWMatrices.Subscribe(mBall->worldTransform, effect->GetPVWMatrixConstant());
    mTrackball.Attach(mBall);
}

void BallHillWindow::CreatePath()
{
    // Create the vertex buffer for the path.  All points are initially at the
    // origin but will be dynamically updated.
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    unsigned int const numVertices = 1024;
    std::shared_ptr<VertexBuffer> vbuffer = std::make_shared<VertexBuffer>(vformat, numVertices);
    vbuffer->SetUsage(Resource::DYNAMIC_UPDATE);
    vbuffer->SetNumActiveElements(0);
    memset(vbuffer->GetData(), 0, vbuffer->GetNumBytes());

    // Create a polyline of contiguous line segments.
    std::shared_ptr<IndexBuffer> ibuffer = std::make_shared<IndexBuffer>(
        IP_POLYSEGMENT_CONTIGUOUS, numVertices - 1);

    // Create a vertex color effect for the path.
    std::shared_ptr<ConstantColorEffect> effect = std::make_shared<ConstantColorEffect>(
        mProgramFactory, Vector4<float>{ 1.0f, 1.0f, 1.0f, 1.0f });
    mPath = std::make_shared<Visual>(vbuffer, ibuffer, effect);

    mPVWMatrices.Subscribe(mPath->worldTransform, effect->GetPVWMatrixConstant());
    mTrackball.Attach(mPath);
}

Vector4<float> BallHillWindow::UpdateBall()
{
    // Compute the location of the center of the ball and the incremental
    // rotation implied by its motion.
    Vector4<float> center;
    Matrix4x4<float> incrRot;
    mModule.GetData(center, incrRot);

    // Update the ball position and orientation.
    mBall->localTransform.SetTranslation(center);
    Matrix4x4<float> orient = mBall->localTransform.GetRotation();
#if defined(GTE_USE_MAT_VEC)
    mBall->localTransform.SetRotation(incrRot * orient);
#else
    mBall->localTransform.SetRotation(orient * incrRot);
#endif

    // Return the new ball center for further use by application.
    return center;
}

void BallHillWindow::PhysicsTick()
{
    // Allow motion only while ball is above the ground level.
    if (mBall->localTransform.GetTranslation()[2] <= mModule.radius)
    {
        return;
    }

    // Move the ball.
    mModule.Update();
    Vector4<float> center = UpdateBall();
    mTrackball.Update();
    mPVWMatrices.Update();

    // Draw only the active quantity of path points for the initial portion
    // of the simulation.  Once all points are activated, then all are drawn.
    std::shared_ptr<VertexBuffer> vbuffer = mPath->GetVertexBuffer();
    unsigned int numVertices = vbuffer->GetNumElements();
    unsigned int numActive = vbuffer->GetNumActiveElements();
    if (numActive < numVertices)
    {
        vbuffer->SetNumActiveElements(++numActive);
        Vector3<float>* position = vbuffer->Get<Vector3<float>>();
        position[numActive] = { center[0], center[1], center[2] };
        if (numActive == 1)
        {
            position[0] = position[1];
        }
        mEngine->Update(vbuffer);
    }
}

void BallHillWindow::GraphicsTick()
{
    mEngine->ClearBuffers();
    mEngine->Draw(mGround);
    mEngine->Draw(mHill);
    mEngine->Draw(mBall);
    mEngine->Draw(mPath);
    mEngine->Draw(8, mYSize - 8, { 1.0f, 1.0f, 1.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);
}
