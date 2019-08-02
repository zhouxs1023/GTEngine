// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2019/04/15)

#include "BlownGlassWindow.h"
#include <Graphics/GteGraphicsDefaults.h>
#include <LowLevel/GteLogReporter.h>

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
        DefaultShaderName("VolumeRender.vs"),
        DefaultShaderName("VolumeRender.ps")
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
    std::string vsPath = mEnvironment.GetPath(DefaultShaderName("VolumeRender.vs"));
    std::string psPath = mEnvironment.GetPath(DefaultShaderName("VolumeRender.ps"));
    auto program = mProgramFactory->CreateFromFiles(vsPath, psPath, "");
    if (!program)
    {
        return false;
    }

    // Create the fluid simulator.
    mFluid = std::make_unique<Fluid3>(mEngine, mProgramFactory, GRID_SIZE, GRID_SIZE, GRID_SIZE, 0.002f);
    mFluid->Initialize();

    // Create the vertex shader for visualization.
    auto cbuffer = std::make_shared<ConstantBuffer>(sizeof(Matrix4x4<float>), true);
    program->GetVShader()->Set("PVWMatrix", cbuffer);
    cbuffer->SetMember("pvwMatrix", Matrix4x4<float>::Identity());

    // Create the pixel shader for visualization.
    auto sampler = std::make_shared<SamplerState>();
    sampler->filter = SamplerState::MIN_L_MAG_L_MIP_P;
    sampler->mode[0] = SamplerState::CLAMP;
    sampler->mode[1] = SamplerState::CLAMP;
    sampler->mode[2] = SamplerState::CLAMP;
    program->GetPShader()->Set("volumeTexture", mFluid->GetState(), "volumeSampler", sampler);

    auto effect = std::make_shared<VisualEffect>(program);

    // Load the level-surface mesh obtained from the SurfaceExtraction sample.
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    unsigned int numVertices = 82832;
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, numVertices);
    path = mEnvironment.GetPath("Vertices82832.raw");
    std::ifstream input(path, std::ios::binary);
    input.read(vbuffer->GetData(), vbuffer->GetNumBytes());
    input.close();

    unsigned int numTriangles = 41388;
    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, numTriangles, sizeof(unsigned int));
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
