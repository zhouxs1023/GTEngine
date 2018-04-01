// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "Fluids3DWindow.h"

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

    Window::Parameters parameters(L"Fluids3DWindow", 0, 0, 768, 768);
    auto window = TheWindowSystem.Create<Fluids3DWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
    TheWindowSystem.Destroy<Fluids3DWindow>(window);
    return 0;
}

Fluids3DWindow::Fluids3DWindow(Parameters& parameters)
    :
    Window3(parameters),
    mFluid(mEngine, mProgramFactory, GRID_SIZE, GRID_SIZE, GRID_SIZE, 0.002f)
{
    if (!SetEnvironment() || !CreateNestedBoxes())
    {
        parameters.created = false;
        return;
    }

    // Use blending for the visualization.
    mAlphaState = std::make_shared<BlendState>();
    mAlphaState->target[0].enable = true;
    mAlphaState->target[0].srcColor = BlendState::BM_SRC_ALPHA;
    mAlphaState->target[0].dstColor = BlendState::BM_INV_SRC_ALPHA;
    mAlphaState->target[0].srcAlpha = BlendState::BM_SRC_ALPHA;
    mAlphaState->target[0].dstAlpha = BlendState::BM_INV_SRC_ALPHA;
    mEngine->SetBlendState(mAlphaState);

    // The alpha channel must be zero for the blending of density to work
    // correctly through the fluid region.
    mEngine->SetClearColor({ 1.0f, 1.0f, 1.0f, 0.0f });

    // The geometric proxies for volume rendering are concentric boxes.  They
    // are drawn from inside to outside for correctly sorted drawing, so depth
    // buffering is not needed.
    mNoDepthState = std::make_shared<DepthStencilState>();
    mNoDepthState->depthEnable = false;
    mEngine->SetDepthStencilState(mNoDepthState);

    mFluid.Initialize();
    InitializeCamera(60.0f, GetAspectRatio(), 0.01f, 100.0f, 0.01f, 0.001f,
        { 0.0f, 0.0f, -2.25f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f });
    mPVWMatrices.Update();

    UpdateConstants();
}

void Fluids3DWindow::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }
    UpdateConstants();

    mFluid.DoSimulationStep();

    mEngine->ClearBuffers();
    for (auto visual : mVisible)
    {
        mEngine->Draw(visual);
    }
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(1);

    mTimer.UpdateFrameCount();
}

bool Fluids3DWindow::OnCharPress(unsigned char key, int x, int y)
{
    if (key == '0')
    {
        mFluid.Initialize();
        return true;
    }

    return Window::OnCharPress(key, x, y);
}

bool Fluids3DWindow::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Physics/Fluids3D/Shaders/");

#if defined(GTE_DEV_OPENGL)
    if (mEnvironment.GetPath("VolumeRenderVS.glsl") == "")
    {
        LogError("Cannot find file VolumeRenderVS.hlsl.");
        return false;
    }
    if (mEnvironment.GetPath("VolumeRenderPS.glsl") == "")
    {
        LogError("Cannot find file VolumeRenderPS.hlsl.");
        return false;
    }
#else
    if (mEnvironment.GetPath("VolumeRender.hlsl") == "")
    {
        LogError("Cannot find file VolumeRender.hlsl.");
        return false;
    }
#endif

    return true;
}

bool Fluids3DWindow::CreateNestedBoxes()
{
#if defined(GTE_DEV_OPENGL)
    std::string pathVS = mEnvironment.GetPath("VolumeRenderVS.glsl");
    std::string pathPS = mEnvironment.GetPath("VolumeRenderPS.glsl");
    std::shared_ptr<VisualProgram> program = mProgramFactory->CreateFromFiles(pathVS, pathPS, "");
#else
    std::string path = mEnvironment.GetPath("VolumeRender.hlsl");
    std::shared_ptr<VisualProgram> program = mProgramFactory->CreateFromFiles(path, path, "");
#endif
    if (!program)
    {
        return false;
    }

    mPVWMatrixBuffer = std::make_shared<ConstantBuffer>(sizeof(Matrix4x4<float>), true);
    program->GetVShader()->Set("PVWMatrix", mPVWMatrixBuffer);
    mPVWMatrixBuffer->SetMember("pvwMatrix", Matrix4x4<float>::Identity());

    mTrilinearClampSampler = std::make_shared<SamplerState>();
    mTrilinearClampSampler->filter = SamplerState::MIN_L_MAG_L_MIP_P;
    mTrilinearClampSampler->mode[0] = SamplerState::CLAMP;
    mTrilinearClampSampler->mode[1] = SamplerState::CLAMP;
    mTrilinearClampSampler->mode[2] = SamplerState::CLAMP;

#if defined(GTE_DEV_OPENGL)
    program->GetPShader()->Set("volumeSampler", mFluid.GetState());
    program->GetPShader()->Set("volumeSampler", mTrilinearClampSampler);
#else
    program->GetPShader()->Set("volumeTexture", mFluid.GetState());
    program->GetPShader()->Set("trilinearClampSampler", mTrilinearClampSampler);
#endif

    std::shared_ptr<VisualEffect> effect = std::make_shared<VisualEffect>(program);

    struct Vertex { Vector3<float> position, tcoord; };
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_TEXCOORD, DF_R32G32B32_FLOAT, 0);

    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    int const numBoxes = 128;
    for (int i = 1; i <= numBoxes; ++i)
    {
        float extent = 0.5f*i/(numBoxes - 1.0f);
        std::shared_ptr<Visual> visual(mf.CreateBox(extent, extent, extent));
        VertexBuffer* vbuffer = visual->GetVertexBuffer().get();
        Vertex* vertex = vbuffer->Get<Vertex>();
        for (unsigned int j = 0; j < vbuffer->GetNumElements(); ++j, ++vertex)
        {
            Vector3<float>& tcd = vertex->tcoord;
            Vector3<float> pos = vertex->position;
            Vector4<float> tmp{ pos[0] + 0.5f, pos[1] + 0.5f, pos[2] + 0.5f, 0.0f };
            for (int k = 0; k < 3; ++k)
            {
                tcd[k] = 0.5f*(tmp[k] + 1.0f);
            }
        }

        visual->SetEffect(effect);
        mVisible.push_back(visual);
    }

    return true;
}

void Fluids3DWindow::UpdateConstants()
{
    Matrix4x4<float> pvMatrix = mCamera->GetProjectionViewMatrix();
    Matrix4x4<float>& pvwMatrix = *mPVWMatrixBuffer->Get<Matrix4x4<float>>();

#if defined(GTE_USE_MAT_VEC)
    pvwMatrix = pvMatrix * mTrackball.GetOrientation();
#else
    pvwMatrix = mTrackball.GetOrientation() * pvMatrix;
#endif

    mEngine->Update(mPVWMatrixBuffer);
}
