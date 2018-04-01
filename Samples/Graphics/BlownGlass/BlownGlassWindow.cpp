// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "BlownGlassWindow.h"

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

    Window::Parameters parameters(L"BlownGlassWindow", 0, 0, 512, 512);
    auto window = TheWindowSystem.Create<BlownGlassWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
    TheWindowSystem.Destroy<BlownGlassWindow>(window);
    return 0;
}

BlownGlassWindow::BlownGlassWindow(Parameters& parameters)
    :
    Window3(parameters)
{
    if (!SetEnvironment() || !CreateScene())
    {
        parameters.created = false;
        return;
    }

    // Use blending for the visualization.
    mMeshBlendState = std::make_shared<BlendState>();
    mMeshBlendState->target[0].enable = true;
    mMeshBlendState->target[0].srcColor = BlendState::BM_SRC_ALPHA;
    mMeshBlendState->target[0].dstColor = BlendState::BM_INV_SRC_ALPHA;
    mMeshBlendState->target[0].srcAlpha = BlendState::BM_SRC_ALPHA;
    mMeshBlendState->target[0].dstAlpha = BlendState::BM_INV_SRC_ALPHA;

    // The alpha channel must be zero for the blending of density to work
    // correctly through the fluid region.
    mEngine->SetClearColor({ 1.0f, 1.0f, 1.0f, 0.0f });

    // Disable face culling.
    mMeshRasterizerState = std::make_shared<RasterizerState>();
    mMeshRasterizerState->cullMode = RasterizerState::CULL_NONE;

    // Read the depth buffer but do not write to it.
    mMeshDepthStencilState = std::make_shared<DepthStencilState>();
    mMeshDepthStencilState->writeMask = DepthStencilState::MASK_ZERO;

    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0f, 0.01f, 0.001f,
        { 2.5f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });
    mPVWMatrices.Update();
}

void BlownGlassWindow::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mFluid->DoSimulationStep();

    mEngine->ClearBuffers();

    mEngine->SetBlendState(mMeshBlendState);
    mEngine->SetRasterizerState(mMeshRasterizerState);
    mEngine->SetDepthStencilState(mMeshDepthStencilState);
    mEngine->Draw(mMesh);
    mEngine->SetDefaultDepthStencilState();
    mEngine->SetDefaultRasterizerState();
    mEngine->SetDefaultBlendState();

    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(1);

    mTimer.UpdateFrameCount();
}

bool BlownGlassWindow::SetEnvironment()
{
    // Set the search path to find images to load.
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }
    
    mEnvironment.Insert(path + "/Samples/Graphics/BlownGlass/Shaders/");
    mEnvironment.Insert(path + "/Samples/Graphics/BlownGlass/Geometry/");

    std::vector<std::string> inputs =
    {
        "Vertices82832.raw",
        "Indices41388.raw",
#if defined(GTE_DEV_OPENGL)
        "VolumeRenderVS.glsl",
        "VolumeRenderPS.glsl"
#else
        "VolumeRender.hlsl"
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

bool BlownGlassWindow::CreateScene()
{
    std::string path;

    // Create the shaders.
#if defined(GTE_DEV_OPENGL)
    std::string const pathVS = mEnvironment.GetPath("VolumeRenderVS.glsl");
    std::string const pathPS = mEnvironment.GetPath("VolumeRenderPS.glsl");
    std::shared_ptr<VisualProgram> program =
        mProgramFactory->CreateFromFiles(pathVS, pathPS, "");
#else
    path = mEnvironment.GetPath("VolumeRender.hlsl");
    std::shared_ptr<VisualProgram> program =
        mProgramFactory->CreateFromFiles(path, path, "");
#endif
    if (!program)
    {
        return false;
    }

    // Create the fluid simulator.
    mFluid = std::make_unique<Fluid3>(mEngine, mProgramFactory, GRID_SIZE,
        GRID_SIZE, GRID_SIZE, 0.002f);
    mFluid->Initialize();

    // Create the vertex shader for visualization.
    std::shared_ptr<ConstantBuffer> cbuffer =
        std::make_shared<ConstantBuffer>(sizeof(Matrix4x4<float>), true);
    program->GetVShader()->Set("PVWMatrix", cbuffer);
    cbuffer->SetMember("pvwMatrix", Matrix4x4<float>::Identity());

    // Create the pixel shader for visualization.
    std::shared_ptr<SamplerState> sampler = std::make_shared<SamplerState>();
    sampler->filter = SamplerState::MIN_L_MAG_L_MIP_P;
    sampler->mode[0] = SamplerState::CLAMP;
    sampler->mode[1] = SamplerState::CLAMP;
    sampler->mode[2] = SamplerState::CLAMP;
#if defined(GTE_DEV_OPENGL)
    program->GetPShader()->Set("volumeSampler", mFluid->GetState());
    program->GetPShader()->Set("volumeSampler", sampler);
#else
    program->GetPShader()->Set("volumeTexture", mFluid->GetState());
    program->GetPShader()->Set("trilinearClampSampler", sampler);
#endif

    std::shared_ptr<VisualEffect> effect = std::make_shared<VisualEffect>(program);

    // Load the level-surface mesh obtained from the SurfaceExtraction sample.
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    unsigned int numVertices = 82832;
    std::shared_ptr<VertexBuffer> vbuffer =
        std::make_shared<VertexBuffer>(vformat, numVertices);
    path = mEnvironment.GetPath("Vertices82832.raw");
    std::ifstream input(path, std::ios::binary);
    input.read(vbuffer->GetData(), vbuffer->GetNumBytes());
    input.close();

    unsigned int numTriangles = 41388;
    std::shared_ptr<IndexBuffer> ibuffer =
        std::make_shared<IndexBuffer>(IP_TRIMESH, numTriangles, sizeof(unsigned int));
    path = mEnvironment.GetPath("Indices41388.raw");
    input.open(path, std::ios::in | std::ios::binary);
    input.read(ibuffer->GetData(), ibuffer->GetNumBytes());
    input.close();

    mMesh = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mMesh->localTransform.SetTranslation(-1.0f, -1.0f, -1.0f);

    // Automatic update of transforms for virtual trackball.
    mPVWMatrices.Subscribe(mMesh->worldTransform, cbuffer);

    mTrackball.Attach(mMesh);
    mTrackball.Update();
    return true;
}
