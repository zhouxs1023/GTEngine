// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2017/09/16)

#include "CubeMapsWindow.h"

int main(int, char const*[])
{
#if defined(_DEBUG)
    // TODO: The message-box logger is not enabled.  The initial draw calls
    // for the 6 walls generate OpenGL errors in the glGetUniformLocation of
    // GL4Engine::EnableTextures.  The errors are GL_INVALID_VALUE, but yet
    // the program handles are valid.  On subsequent draw calls, the OpenGL
    // errors are NOT generated.  This behavior occurs on AMD or NVIDIA
    // hardware.  We need to diagnose the problem.
    LogReporter reporter(
        "LogReport.txt",
        Logger::Listener::LISTEN_FOR_ALL,
        Logger::Listener::LISTEN_FOR_ALL,
        Logger::Listener::LISTEN_FOR_NOTHING,
        Logger::Listener::LISTEN_FOR_ALL);
#endif

    Window::Parameters parameters(L"CubeMapsWindow", 0, 0, 512, 512);
    auto window = TheWindowSystem.Create<CubeMapsWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
    TheWindowSystem.Destroy<CubeMapsWindow>(window);
    return 0;
}

CubeMapsWindow::CubeMapsWindow(Parameters& parameters)
    :
    Window3(parameters)
{
    if (!SetEnvironment() || !CreateCubeMapItems())
    {
        parameters.created = false;
        return;
    }

    mNoCullState = std::make_shared<RasterizerState>();
    mNoCullState->cullMode = RasterizerState::CULL_NONE;

    InitializeCamera(60.0f, GetAspectRatio(), 0.01f, 10.0f, 0.01f, 0.01f,
        { 0.0f, 0.0f, -0.85f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f });

    CreateScene();
    mPVWMatrices.Update();
    mCuller.ComputeVisibleSet(mCamera, mScene);
}

void CubeMapsWindow::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        if (mCubeMapEffect->DynamicUpdates())
        {
            // Cull the sphere object because it is the object that
            // reflects the environment.
            mSphere->culling = CULL_ALWAYS;

            // You can take a snapshot of the environment from any camera
            // position and camera orientation.  In this application, the
            // environment is always rendered from the center of the cube
            // object and using the axes of that cube for the orientation.
            mCubeMapEffect->UpdateFaces(mEngine, mScene, mCuller,
                { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 0.0f },
                { 0.0f, 1.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f, 0.0f });

            // Restore the sphere object's culling state.
            mSphere->culling = CULL_DYNAMIC;
        }

        mPVWMatrices.Update();
        mCuller.ComputeVisibleSet(mCamera, mScene);
    }

    mCubeMapEffect->SetWMatrix(mSphere->worldTransform);
    mEngine->Update(mCubeMapEffect->GetWMatrixConstant());
    mCubeMapEffect->SetCameraWorldPosition(mCamera->GetPosition());
    mEngine->Update(mCubeMapEffect->GetCameraWorldPositionConstant());

    mEngine->ClearBuffers();
    for (auto const& visual : mCuller.GetVisibleSet())
    {
        mEngine->Draw(visual);
    }
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool CubeMapsWindow::OnCharPress(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'c':
    case 'C':
        if (mEngine->GetRasterizerState() == mNoCullState)
        {
            mEngine->SetDefaultRasterizerState();
        }
        else
        {
            mEngine->SetRasterizerState(mNoCullState);
        }
        return true;
    }

    return Window3::OnCharPress(key, x, y);
}

bool CubeMapsWindow::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Graphics/CubeMaps/Shaders/");
    mEnvironment.Insert(path + "/Samples/Data/");

    std::vector<std::string> inputs =
    {
        "XmFace.png",
        "XpFace.png",
        "YmFace.png",
        "YpFace.png",
        "ZmFace.png",
        "ZpFace.png",
#if defined(GTE_DEV_OPENGL)
        "CubeMapVS.glsl",
        "CubeMapPS.glsl"
#else
        "CubeMap.hlsl"
#endif
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

bool CubeMapsWindow::CreateCubeMapItems()
{
    std::string name[6] =
    {
        "XpFace.png",
        "XmFace.png",
        "YpFace.png",
        "YmFace.png",
        "ZpFace.png",
        "ZmFace.png"
    };

    // The cube-map faces are 64x64 textures.
    mCubeTexture = std::make_shared<TextureCube>(DF_R8G8B8A8_UNORM, 64, true);
    mCubeTexture->AutogenerateMipmaps();
    mCubeTexture->SetCopyType(Resource::COPY_CPU_TO_STAGING);
    for (int face = 0; face < 6; ++face)
    {
        std::string textureName = mEnvironment.GetPath(name[face]);
        auto texture = WICFileIO::Load(textureName, true);
        memcpy(mCubeTexture->GetDataFor(face, 0), texture->GetData(), texture->GetNumBytes());
    }

    bool created = false;
    mCubeMapEffect = std::make_shared<CubeMapEffect>(mProgramFactory, mEnvironment,
        mCubeTexture, 0.5f, created);
    if (created)
    {
        // The inputs are the same as those used to create Window3::mCamera in
        // the CubeMapsWindow constructor.
        mCubeMapEffect->UseDynamicUpdates(0.01f, 10.0f);
    }
    return created;
}

void CubeMapsWindow::CreateScene()
{
    // Create the root of the scene.
    mScene = std::make_shared<Node>();

    // Create the walls of the cube room.  Each of the six texture images is
    // RGBA 64-by-64.
    std::shared_ptr<Node> room = std::make_shared<Node>();
    mScene->AttachChild(room);

    // The vertex format shared by the room walls.
    struct Vertex
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);

    // The index buffer shared by the room walls.
    std::shared_ptr<IndexBuffer> ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH,
        2, sizeof(unsigned int));
    unsigned int* indices = ibuffer->Get<unsigned int>();
    indices[0] = 0;  indices[1] = 1;  indices[2] = 3;
    indices[3] = 0;  indices[4] = 3;  indices[5] = 2;

    std::shared_ptr<VertexBuffer> vbuffer;
    Vertex* vertex;
    std::shared_ptr<Texture2> texture;
    std::shared_ptr<Texture2Effect> effect;
    std::shared_ptr<Visual> wall;
    SamplerState::Filter filter = SamplerState::MIN_L_MAG_L_MIP_L;
    SamplerState::Mode mode = SamplerState::WRAP;

    // +x wall
    vbuffer = std::make_shared<VertexBuffer>(vformat, 4);
    vertex = vbuffer->Get<Vertex>();
    vertex[0] = { { +1.0f, -1.0f, -1.0f }, { 1.0f, 1.0f } };
    vertex[1] = { { +1.0f, -1.0f, +1.0f }, { 0.0f, 1.0f } };
    vertex[2] = { { +1.0f, +1.0f, -1.0f }, { 1.0f, 0.0f } };
    vertex[3] = { { +1.0f, +1.0f, +1.0f }, { 0.0f, 0.0f } };
    texture = WICFileIO::Load(mEnvironment.GetPath("XpFace.png"), true);
    effect = std::make_shared<Texture2Effect>(mProgramFactory, texture, filter, mode, mode);
    wall = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    wall->UpdateModelBound();
    room->AttachChild(wall);
    mPVWMatrices.Subscribe(wall->worldTransform, effect->GetPVWMatrixConstant());
    wall->name = "+x wall";

    // -x wall
    vbuffer = std::make_shared<VertexBuffer>(vformat, 4);
    vertex = vbuffer->Get<Vertex>();
    vertex[0] = { { -1.0f, -1.0f, +1.0f },{ 1.0f, 1.0f } };
    vertex[1] = { { -1.0f, -1.0f, -1.0f },{ 0.0f, 1.0f } };
    vertex[2] = { { -1.0f, +1.0f, +1.0f },{ 1.0f, 0.0f } };
    vertex[3] = { { -1.0f, +1.0f, -1.0f },{ 0.0f, 0.0f } };
    texture = WICFileIO::Load(mEnvironment.GetPath("XmFace.png"), true);
    effect = std::make_shared<Texture2Effect>(mProgramFactory, texture, filter, mode, mode);
    wall = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    wall->UpdateModelBound();
    room->AttachChild(wall);
    mPVWMatrices.Subscribe(wall->worldTransform, effect->GetPVWMatrixConstant());
    wall->name = "-x wall";

    // +y wall
    vbuffer = std::make_shared<VertexBuffer>(vformat, 4);
    vertex = vbuffer->Get<Vertex>();
    vertex[0] = { { +1.0f, +1.0f, +1.0f },{ 1.0f, 1.0f } };
    vertex[1] = { { -1.0f, +1.0f, +1.0f },{ 0.0f, 1.0f } };
    vertex[2] = { { +1.0f, +1.0f, -1.0f },{ 1.0f, 0.0f } };
    vertex[3] = { { -1.0f, +1.0f, -1.0f },{ 0.0f, 0.0f } };
    texture = WICFileIO::Load(mEnvironment.GetPath("YpFace.png"), true);
    effect = std::make_shared<Texture2Effect>(mProgramFactory, texture, filter, mode, mode);
    wall = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    wall->UpdateModelBound();
    room->AttachChild(wall);
    mPVWMatrices.Subscribe(wall->worldTransform, effect->GetPVWMatrixConstant());
    wall->name = "+y wall";

    // -y wall
    vbuffer = std::make_shared<VertexBuffer>(vformat, 4);
    vertex = vbuffer->Get<Vertex>();
    vertex[0] = { { +1.0f, -1.0f, -1.0f },{ 1.0f, 1.0f } };
    vertex[1] = { { -1.0f, -1.0f, -1.0f },{ 0.0f, 1.0f } };
    vertex[2] = { { +1.0f, -1.0f, +1.0f },{ 1.0f, 0.0f } };
    vertex[3] = { { -1.0f, -1.0f, +1.0f },{ 0.0f, 0.0f } };
    texture = WICFileIO::Load(mEnvironment.GetPath("YmFace.png"), true);
    effect = std::make_shared<Texture2Effect>(mProgramFactory, texture, filter, mode, mode);
    wall = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    wall->UpdateModelBound();
    room->AttachChild(wall);
    mPVWMatrices.Subscribe(wall->worldTransform, effect->GetPVWMatrixConstant());
    wall->name = "-y wall";

    // +z wall
    vbuffer = std::make_shared<VertexBuffer>(vformat, 4);
    vertex = vbuffer->Get<Vertex>();
    vertex[0] = { { +1.0f, -1.0f, +1.0f },{ 1.0f, 1.0f } };
    vertex[1] = { { -1.0f, -1.0f, +1.0f },{ 0.0f, 1.0f } };
    vertex[2] = { { +1.0f, +1.0f, +1.0f },{ 1.0f, 0.0f } };
    vertex[3] = { { -1.0f, +1.0f, +1.0f },{ 0.0f, 0.0f } };
    texture = WICFileIO::Load(mEnvironment.GetPath("ZpFace.png"), true);
    effect = std::make_shared<Texture2Effect>(mProgramFactory, texture, filter, mode, mode);
    wall = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    wall->UpdateModelBound();
    room->AttachChild(wall);
    mPVWMatrices.Subscribe(wall->worldTransform, effect->GetPVWMatrixConstant());
    wall->name = "+z wall";

    // -z wall
    vbuffer = std::make_shared<VertexBuffer>(vformat, 4);
    vertex = vbuffer->Get<Vertex>();
    vertex[0] = { { -1.0f, -1.0f, -1.0f },{ 1.0f, 1.0f } };
    vertex[1] = { { +1.0f, -1.0f, -1.0f },{ 0.0f, 1.0f } };
    vertex[2] = { { -1.0f, +1.0f, -1.0f },{ 1.0f, 0.0f } };
    vertex[3] = { { +1.0f, +1.0f, -1.0f },{ 0.0f, 0.0f } };
    texture = WICFileIO::Load(mEnvironment.GetPath("ZmFace.png"), true);
    effect = std::make_shared<Texture2Effect>(mProgramFactory, texture, filter, mode, mode);
    wall = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    wall->UpdateModelBound();
    room->AttachChild(wall);
    mPVWMatrices.Subscribe(wall->worldTransform, effect->GetPVWMatrixConstant());
    wall->name = "-z wall";

    // A sphere to reflect the environment via a cube map.  The colors will
    // be used to modulate the cube map texture.
    struct SVertex
    {
        Vector3<float> position, normal, color;
    };
    VertexFormat svformat;
    svformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    svformat.Bind(VA_NORMAL, DF_R32G32B32_FLOAT, 0);
    svformat.Bind(VA_COLOR, DF_R32G32B32_FLOAT, 0);

    MeshFactory mf;
    mf.SetVertexFormat(svformat);
    mSphere = mf.CreateSphere(64, 64, 0.125f);
    mSphere->UpdateModelBound();
    room->AttachChild(mSphere);

    // Generate random vertex colors for the sphere.  The StandardMesh class
    // produces a sphere with duplicated vertices along a longitude line.
    // This allows texture coordinates to be assigned in a manner that treats
    // the sphere as if it were a rectangle mesh.  For vertex colors, we want
    // the duplicated vertices to have the same color, so a hash table is used
    // to look up vertex colors for the duplicates.
    std::mt19937 mte;
    std::uniform_real_distribution<float> rndG(0.5f, 0.75f);
    std::uniform_real_distribution<float> rndB(0.75f, 1.0f);
    vbuffer = mSphere->GetVertexBuffer();
    unsigned int const numVertices = vbuffer->GetNumElements();
    SVertex* svertex = vbuffer->Get<SVertex>();
    std::map<Vector3<float>, Vector3<float>> dataMap;
    for (unsigned int i = 0; i < numVertices; ++i)
    {
        auto const& element = dataMap.find(svertex[i].position);
        if (element != dataMap.end())
        {
            svertex[i].color = element->second;
        }
        else
        {
            svertex[i].color = { 0.0f, rndG(mte), rndB(mte) };
        }
    }

    mSphere->SetEffect(mCubeMapEffect);
    mPVWMatrices.Subscribe(mSphere->worldTransform, mCubeMapEffect->GetPVWMatrixConstant());

    mScene->Update();
}
