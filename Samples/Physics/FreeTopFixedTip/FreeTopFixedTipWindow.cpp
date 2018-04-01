// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.1.0 (2016/06/28)

#include "FreeTopFixedTipWindow.h"

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

    Window::Parameters parameters(L"FreeTopFixedTipWindow", 0, 0, 640, 480);
    auto window = TheWindowSystem.Create<FreeTopFixedTipWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
    TheWindowSystem.Destroy<FreeTopFixedTipWindow>(window);
    return 0;
}

FreeTopFixedTipWindow::FreeTopFixedTipWindow(Parameters& parameters)
    :
    Window3(parameters),
    mMaxPhi(0.0f),
    mLastUpdateTime(mMotionTimer.GetSeconds())
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    mWireState = std::make_shared<RasterizerState>();
    mWireState->fillMode = RasterizerState::FILL_WIREFRAME;

    CreateScene();
    float angle = static_cast<float>(0.1 * GTE_C_PI);
    float cs = cos(angle), sn = sin(angle);
    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0f, 0.001f, 0.001f,
        { 4.0f, 0.0f, 2.0f }, { -cs, 0.0f, -sn }, { -sn, 0.0f, cs });
    mPVWMatrices.Update();
}

void FreeTopFixedTipWindow::OnIdle()
{
    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    double time = mMotionTimer.GetSeconds();
    if (30.0 * (time - mLastUpdateTime) >= 1.0)
    {
        mLastUpdateTime = time;
#if !defined(FREE_TOP_FIXED_TIP_SINGLE_STEP)
        PhysicsTick();
#endif
        mTrackball.Update();
        GraphicsTick();
    }
}

bool FreeTopFixedTipWindow::OnCharPress(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'w':
    case 'W':
        if (mWireState == mEngine->GetRasterizerState())
        {
            mEngine->SetDefaultRasterizerState();
        }
        else
        {
            mEngine->SetRasterizerState(mWireState);
        }
        return true;

    case 'i':
    case 'I':
        InitializeModule();
        return true;

#if defined(FREE_TOP_FIXED_TIP_SINGLE_STEP)
    case 'g':
    case 'G':
        PhysicsTick();
        return true;
#endif
    }

    return Window3::OnCharPress(key, x, y);
}

bool FreeTopFixedTipWindow::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");
    std::vector<std::string> inputs =
    {
        "Wood.png",
        "TopTexture.png"
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

void FreeTopFixedTipWindow::InitializeModule()
{
    mModule.gravity = 10.0f;
    mModule.mass = 1.0f;
    mModule.length = 8.0f;
    mModule.inertia1 = 1.0f;
    mModule.inertia3 = 2.0f;

    float time = 0.0f;
    float deltaTime = 0.01f;
    float theta = 0.0f;
    float phi = 0.001f;
    float psi = 0.0f;
    float angVel1 = 1.0f;
    float angVel2 = 0.0f;
    float angVel3 = 10.0f;
    mModule.Initialize(time, deltaTime, theta, phi, psi, angVel1, angVel2, angVel3);
    mMaxPhi = static_cast<float>(GTE_C_HALF_PI - atan(2.0 / 3.0));
}

void FreeTopFixedTipWindow::CreateScene()
{
    // scene -+--- floor
    //        |
    //        +--- vertical axis
    //        |
    //        +--- top root ---+--- top
    //                         |
    //                         +--- top axis

    InitializeModule();

    mScene = std::make_shared<Node>();
    mTopRoot = std::make_shared<Node>();
    mScene->AttachChild(mTopRoot);
    CreateFloor();
    CreateAxisVertical();
    CreateTop();
    CreateAxisTop();
    mTrackball.Attach(mScene);
    mTrackball.Update();
}

void FreeTopFixedTipWindow::CreateFloor()
{
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    auto visual = mf.CreateRectangle(2, 2, 32.0f, 32.0f);

    auto texture = WICFileIO::Load(mEnvironment.GetPath("Wood.png"), true);
    texture->AutogenerateMipmaps();
    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::MIN_L_MAG_L_MIP_L, SamplerState::CLAMP, SamplerState::CLAMP);

    visual->SetEffect(effect);
    mPVWMatrices.Subscribe(visual->worldTransform, effect->GetPVWMatrixConstant());
    mVisuals.push_back(visual);
    mScene->AttachChild(visual);
}

void FreeTopFixedTipWindow::CreateAxisVertical()
{
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, 2);
    Vector3<float>* vertices = vbuffer->Get<Vector3<float>>();
    vertices[0] = { 0.0f, 0.0f, 0.0f };
    vertices[1] = { 0.0f, 0.0f, 4.0f };

    auto ibuffer = std::make_shared<IndexBuffer>(IP_POLYSEGMENT_DISJOINT, 1);

    Vector4<float> black{ 0.0f, 0.0f, 0.0f, 1.0f };
    auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory, black);

    auto visual = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    visual->SetEffect(effect);
    mPVWMatrices.Subscribe(visual->worldTransform, effect->GetPVWMatrixConstant());
    mVisuals.push_back(visual);
    mScene->AttachChild(visual);
}

void FreeTopFixedTipWindow::CreateTop()
{
    struct Vertex
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };

    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    auto visual = mf.CreateCylinderOpen(32, 32, 1.0f, 2.0f);
    visual->localTransform.SetTranslation(0.0f, 0.0f, 1.0f);

    // Adjust the shape.
    auto vbuffer = visual->GetVertexBuffer();
    unsigned int numVertices = vbuffer->GetNumElements();
    Vertex* vertices = vbuffer->Get<Vertex>();
    for (unsigned int i = 0; i < numVertices; ++i)
    {
        Vector3<float>& pos = vertices[i].position;
        float z = pos[2] + 1.0f;
        float r = 0.75f * (z >= 1.5f ? 4.0f - 2.0f * z : z / 1.5f);
        float multiplier = r / sqrt(pos[0] * pos[0] + pos[1] * pos[1]);
        pos[0] *= multiplier;
        pos[1] *= multiplier;
        vertices[i].tcoord *= 4.0f;
    }

    auto texture = WICFileIO::Load(mEnvironment.GetPath("TopTexture.png"), true);
    texture->AutogenerateMipmaps();
    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::MIN_L_MAG_L_MIP_L, SamplerState::WRAP, SamplerState::WRAP);

    visual->SetEffect(effect);
    mPVWMatrices.Subscribe(visual->worldTransform, effect->GetPVWMatrixConstant());
    mVisuals.push_back(visual);
    mTopRoot->AttachChild(visual);
}

void FreeTopFixedTipWindow::CreateAxisTop()
{
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, 2);
    Vector3<float>* vertices = vbuffer->Get<Vector3<float>>();
    vertices[0] = { 0.0f, 0.0f, 0.0f };
    vertices[1] = { 0.0f, 0.0f, 4.0f };

    auto ibuffer = std::make_shared<IndexBuffer>(IP_POLYSEGMENT_DISJOINT, 1);

    Vector4<float> white{ 1.0f, 1.0f, 1.0f, 1.0f };
    auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory, white);

    auto visual = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    visual->SetEffect(effect);
    mPVWMatrices.Subscribe(visual->worldTransform, effect->GetPVWMatrixConstant());
    mVisuals.push_back(visual);
    mTopRoot->AttachChild(visual);
}

void FreeTopFixedTipWindow::PhysicsTick()
{
    // Stop the simulation when the top edge reaches the ground.
    if (mModule.GetPhi() >= mMaxPhi)
    {
        // EXERCISE.  Instead of stopping the top, maintain its phi value at
        // mMaxPhi so that the top continues to roll on the ground.  In
        // addition, arrange for the top to slow down while rolling on the
        // ground, eventually coming to a stop.
        return;
    }

    // Move the top
    mModule.Update();
    mTopRoot->localTransform.SetRotation(mModule.GetBodyAxes());
    mTopRoot->Update();
    mPVWMatrices.Update();
}

void FreeTopFixedTipWindow::GraphicsTick()
{
    mTimer.Measure();

    mEngine->ClearBuffers();
    for (auto const& visual : mVisuals)
    {
        mEngine->Draw(visual);
    }
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}
