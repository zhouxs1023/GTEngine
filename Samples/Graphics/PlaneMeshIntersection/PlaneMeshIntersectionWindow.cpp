// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "PlaneMeshIntersectionWindow.h"

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

    Window::Parameters parameters(L"PlaneMeshIntersectionWindow", 0, 0, 512, 512);
    auto window = TheWindowSystem.Create<PlaneMeshIntersectionWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
    TheWindowSystem.Destroy<PlaneMeshIntersectionWindow>(window);
    return 0;
}

PlaneMeshIntersectionWindow::PlaneMeshIntersectionWindow(
    Parameters& parameters)
    :
    Window3(parameters)
{
    if (!SetEnvironment() || !CreateScene())
    {
        parameters.created = false;
        return;
    }

    mPSTarget = std::make_shared<DrawTarget>(2, DF_R32G32B32A32_FLOAT,
        mXSize, mYSize, true, false, DF_D24_UNORM_S8_UINT, false);
    mPSColor = mPSTarget->GetRTTexture(0);
    mPSPlaneConstant = mPSTarget->GetRTTexture(1);

    mScreen = std::make_shared<Texture2>(DF_R32G32B32A32_FLOAT, mXSize, mYSize);
    mScreen->SetUsage(Resource::SHADER_OUTPUT);
    mScreen->SetCopyType(Resource::COPY_STAGING_TO_CPU);

    mOverlay = std::make_shared<OverlayEffect>(mProgramFactory, mXSize, mYSize, mXSize, mYSize,
        SamplerState::MIN_P_MAG_P_MIP_P, SamplerState::CLAMP, SamplerState::CLAMP, true);
    mOverlay->SetTexture(mScreen);

    mEngine->SetClearColor({ 1.0f, 1.0f, 1.0f, std::numeric_limits<float>::max() });

    std::shared_ptr<ComputeShader> cshader = mDrawIntersections->GetCShader();
#if defined(GTE_DEV_OPENGL)
    cshader->Set("colorImage", mPSColor);
    cshader->Set("planeConstantImage", mPSPlaneConstant);
    cshader->Set("outputImage", mScreen);
#else
    cshader->Set("color", mPSColor);
    cshader->Set("planeConstant", mPSPlaneConstant);
    cshader->Set("output", mScreen);
#endif

    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0f, 0.01f, 0.001f,
        {0.0f, 0.0f, -2.5f}, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f });
    mPVWMatrices.Update();
}

void PlaneMeshIntersectionWindow::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }
    UpdateMatrices();

    mEngine->Enable(mPSTarget);
    mEngine->ClearBuffers();
    mEngine->Draw(mMesh);
    mEngine->Disable(mPSTarget);
    mEngine->Execute(mDrawIntersections, mXSize / 8, mYSize / 8, 1);
    mEngine->Draw(mOverlay);
    mEngine->Draw(8, mYSize - 8, { 1.0f, 1.0f, 1.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool PlaneMeshIntersectionWindow::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Graphics/PlaneMeshIntersection/Shaders/");
    std::vector<std::string> inputs =
    {
#if defined(GTE_DEV_OPENGL)
        "PlaneMeshIntersectionVS.glsl",
        "PlaneMeshIntersectionPS.glsl",
        "DrawIntersections.glsl"
#else
        "PlaneMeshIntersection.hlsl",
        "DrawIntersections.hlsl"
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

bool PlaneMeshIntersectionWindow::CreateScene()
{
    std::string path;

#if defined(GTE_DEV_OPENGL)
    std::shared_ptr<VisualProgram> program =
        mProgramFactory->CreateFromFiles(
            mEnvironment.GetPath("PlaneMeshIntersectionVS.glsl"),
            mEnvironment.GetPath("PlaneMeshIntersectionPS.glsl"),
            "");
#else
    path = mEnvironment.GetPath("PlaneMeshIntersection.hlsl");
    std::shared_ptr<VisualProgram> program =
        mProgramFactory->CreateFromFiles(path, path, "");
#endif
    if (!program)
    {
        return false;
    }

#if defined(GTE_DEV_OPENGL)
    path = mEnvironment.GetPath("DrawIntersections.glsl");
#else
    path = mEnvironment.GetPath("DrawIntersections.hlsl");
#endif
    mDrawIntersections = mProgramFactory->CreateFromFile(path);
    if (!mDrawIntersections)
    {
        return false;
    }

    float planeDelta = 0.125f;
    mPMIParameters = std::make_shared<ConstantBuffer>(sizeof(PMIParameters), true);
    PMIParameters& p = *mPMIParameters->Get<PMIParameters>();
    p.pvMatrix = mCamera->GetProjectionViewMatrix();
    p.wMatrix = Matrix4x4<float>::Identity();
    p.planeVector0 = Vector4<float>{ 1.0f, 0.0f, 0.0f, 0.0f } / planeDelta;
    p.planeVector1 = Vector4<float>{ 0.0f, 1.0f, 0.0f, 0.0f } / planeDelta;
    program->GetVShader()->Set("PMIParameters", mPMIParameters);

    std::shared_ptr<VisualEffect> effect = std::make_shared<VisualEffect>(program);

    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    mMesh = mf.CreateSphere(16, 16, 1.0f);
    mMesh->SetEffect(effect);
    mMesh->Update();
    return true;
}

void PlaneMeshIntersectionWindow::UpdateMatrices()
{
    PMIParameters& p = *mPMIParameters->Get<PMIParameters>();
    p.pvMatrix = mCamera->GetProjectionViewMatrix();

#if defined(GTE_USE_MAT_VEC)
    p.wMatrix = mTrackball.GetOrientation() * mMesh->worldTransform;
#else
    p.wMatrix = mMesh->worldTransform * mTrackball.GetOrientation();
#endif

    mEngine->Update(mPMIParameters);
}
