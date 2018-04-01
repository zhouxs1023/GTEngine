// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "BlendedAnimationsWindow.h"

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

    Window::Parameters parameters(L"BlendedAnimationsWindow", 0, 0, 768, 768);
    auto window = TheWindowSystem.Create<BlendedAnimationsWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
    TheWindowSystem.Destroy<BlendedAnimationsWindow>(window);
    return 0;
}

BlendedAnimationsWindow::BlendedAnimationsWindow(Parameters& parameters)
    :
    Window3(parameters),
    mUpArrowPressed(false),
    mShiftPressed(false)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    std::string gtePath = mEnvironment.GetVariable("GTE_PATH");
    std::string rootPath = gtePath + "/Samples/Graphics/BlendedAnimations/Data/";
    SkinController::Updater postUpdate =
        [this](std::shared_ptr<VertexBuffer> const& vbuffer)
        {
            mEngine->Update(vbuffer);
        };
    mManager = std::make_unique<BipedManager>(rootPath, "Biped", mProgramFactory, postUpdate);

    // Set animation information.  The counts differ in debug and release
    // builds because of the differing frame rates of those builds.
#if defined(_DEBUG)
    int idleWalkCount = 100;
    int walkCount = 10;
    int walkRunCount = 100;
    mApplicationTime = 0.0;
    mApplicationTimeDelta = 0.01;
#else
    int idleWalkCount = 1000;
    int walkCount = 100;
    int walkRunCount = 1000;
    mApplicationTime = 0.0;
    mApplicationTimeDelta = 0.001;
#endif

    // The idle head turning occurs too frequently (frequency = 1 in the
    // original model).  Reduce the turning by half.
    mManager->SetIdle(0.5, 0.0);

    // The walk and run cycles must be aligned properly for blending.  A
    // phase of 0.2 for the run cycle aligns the biped feet.
    mManager->SetRun(1.0, 0.2);

    // The initial state is 'idle'.
    mManager->Initialize(idleWalkCount, walkCount, walkRunCount);

    CreateScene();

    Vector3<float> dir{ 1.0f, 1.0f, -1.0f }, up{ 0.5f, 0.5f, 1.0f };
    Normalize(dir);
    Normalize(up);
    InitializeCamera(60.0f, GetAspectRatio(), 1.0f, 1000.0f, 0.01f, 0.01f,
        { -60.0f, -60.0f, 90.0f }, { dir[0], dir[1], dir[2] }, { up[0], up[1], up[2] });
    mPVWMatrices.Update();
}

void BlendedAnimationsWindow::OnIdle()
{
    mTimer.Measure();

    Update();

    mEngine->ClearBuffers();
    for (auto const& mesh : mMeshes)
    {
        mEngine->Draw(mesh);
    }

    std::array<float, 4> textColor{ 1.0f, 1.0f, 1.0f, 1.0f };
    mEngine->Draw(8, 24, textColor, "Press UP-ARROW to transition from idle to walk.");
    mEngine->Draw(8, 48, textColor,  "Press SHIFT-UP-ARROW to transition from walk to run.");
    mEngine->Draw(128, mYSize - 8, textColor, "time = " + std::to_string(mApplicationTime));
    mEngine->Draw(8, mYSize - 8, textColor, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool BlendedAnimationsWindow::OnCharPress(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'w':
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
    }

    return Window::OnCharPress(key, x, y);
}

bool BlendedAnimationsWindow::OnKeyDown(int key, int, int)
{
    if (key == KEY_UP)
    {
        mUpArrowPressed = true;
    }
    else if (key == KEY_SHIFT)
    {
        mShiftPressed = true;
    }

    // Window3::OnKeyDown is not called to prevent camera motion.
    return true;
}

bool BlendedAnimationsWindow::OnKeyUp(int key, int, int)
{
    if (key == KEY_UP)
    {
        mUpArrowPressed = false;
    }
    else if (key == KEY_SHIFT)
    {
        mShiftPressed = false;
    }

    // Window3::OnKeyUp is not called to prevent camera motion.
    return true;
}

bool BlendedAnimationsWindow::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");
    mEnvironment.Insert(path + "/Samples/Graphics/BlendedAnimations/Data/");
    if (mEnvironment.GetPath("Grating.png") == "")
    {
        LogError("Cannot find file Grating.png.");
        return false;
    }

    return true;
}

void BlendedAnimationsWindow::CreateScene()
{
    mWireState = std::make_shared<RasterizerState>();
    mWireState->fillMode = RasterizerState::FILL_WIREFRAME;

    mScene = std::make_shared<Node>();
    mScene->AttachChild(mManager->GetRoot());

    // Create a floor to walk on.
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);

    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    mFloor = mf.CreateRectangle(2, 2, 1024.0f, 2048.0f);
    mFloor->name = "Floor";
    mScene->AttachChild(mFloor);
    std::shared_ptr<VertexBuffer> vbuffer = mFloor->GetVertexBuffer();
    vbuffer->SetUsage(Resource::DYNAMIC_UPDATE);
    unsigned int numVertices = vbuffer->GetNumElements();
    Vertex* vertex = vbuffer->Get<Vertex>();
    for (unsigned int i = 0; i < numVertices; ++i)
    {
        vertex[i].tcoord[0] *= 64.0f;
        vertex[i].tcoord[1] *= 256.0f;
    }

    std::string textureName = mEnvironment.GetPath("Grating.png");
    std::shared_ptr<Texture2> texture = WICFileIO::Load(textureName, true);
    texture->AutogenerateMipmaps();
    std::shared_ptr<Texture2Effect> effect = std::make_shared<Texture2Effect>(
        mProgramFactory, texture, SamplerState::MIN_L_MAG_L_MIP_L,
        SamplerState::WRAP, SamplerState::WRAP);
    mFloor->SetEffect(effect);

    mPVWMatrices.Subscribe(mFloor->worldTransform, effect->GetPVWMatrixConstant());
    for (auto const& subscriber : mManager->GetSubscribers())
    {
        mPVWMatrices.Subscribe(subscriber.first->worldTransform, subscriber.second);
    }

    GetMeshes(mScene);

    mTrackball.Attach(mScene);
    mTrackball.Update(mApplicationTime);
}

void BlendedAnimationsWindow::GetMeshes(
    std::shared_ptr<Spatial> const& object)
{
    Visual* mesh = dynamic_cast<Visual*>(object.get());
    if (mesh)
    {
        mMeshes.push_back(mesh);
        return;
    }

    Node* node = dynamic_cast<Node*>(object.get());
    if (node)
    {
        for (int i = 0; i < node->GetNumChildren(); ++i)
        {
            auto const& child = node->GetChild(i);
            if (child)
            {
                GetMeshes(child);
            }
        }
    }
}

void BlendedAnimationsWindow::Update()
{
    // Animate the biped.
    mManager->Update(mUpArrowPressed, mShiftPressed);
    mScene->Update(mApplicationTime);
    mApplicationTime += mApplicationTimeDelta;

    // Give the impression the floor is moving by translating the texture
    // coordinates.  For this demo, the texture coordinates are modified in
    // the vertex buffer.  Generally, you could write a vertex shader with
    // time and velocity as inputs, and then compute the new texture
    // coordinates in the shader.
#if defined(_DEBUG)
    float adjust = mManager->GetSpeed() / 16.0f;
#else
    float adjust = mManager->GetSpeed() / 160.0f;
#endif
    std::shared_ptr<VertexBuffer> vbuffer = mFloor->GetVertexBuffer();
    unsigned int numVertices = vbuffer->GetNumElements();
    Vertex* vertex = vbuffer->Get<Vertex>();
    for (unsigned int i = 0; i < numVertices; ++i)
    {
        vertex[i].tcoord[1] -= adjust;
    }
    mEngine->Update(vbuffer);
}
