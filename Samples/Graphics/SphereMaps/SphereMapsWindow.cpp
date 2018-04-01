// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "SphereMapsWindow.h"

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

    Window::Parameters parameters(L"SphereMapsWindow", 0, 0, 640, 480);
    auto window = TheWindowSystem.Create<SphereMapsWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
    TheWindowSystem.Destroy<SphereMapsWindow>(window);
    return 0;
}

SphereMapsWindow::SphereMapsWindow(Parameters& parameters)
    :
    Window3(parameters)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    // Center the objects in the view frustum.
    CreateScene();
    mScene->localTransform.SetTranslation(-mScene->worldBound.GetCenter());
    float y = -2.0f * mScene->worldBound.GetRadius();
    InitializeCamera(60.0f, GetAspectRatio(), 1.0f, 1000.0f, 0.001f, 0.001f,
        { 0.0f, y, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });

    mTrackball.Update();
    mPVWMatrices.Update();
}

void SphereMapsWindow::OnIdle()
{
    mTimer.Measure();

    mCameraRig.Move();
    UpdateConstants();

    mEngine->ClearBuffers();
    mEngine->Draw(mTorus);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool SphereMapsWindow::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");

    if (mEnvironment.GetPath("SphereMap.png") == "")
    {
        LogError("Cannot find file SphereMap.png");
        return false;
    }

    return true;
}

void SphereMapsWindow::CreateScene()
{
    mScene = std::make_shared<Node>();

    struct Vertex
    {
        Vector3<float> position, normal;
    };

    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_NORMAL, DF_R32G32B32_FLOAT, 0);

    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    mTorus = mf.CreateTorus(64, 64, 1.0f, 0.5f);

    std::string path = mEnvironment.GetPath("SphereMap.png");
    std::shared_ptr<Texture2> texture = WICFileIO::Load(path, false);
    mSMEffect = std::make_shared<SphereMapEffect>(mProgramFactory, texture,
        SamplerState::MIN_L_MAG_L_MIP_P, SamplerState::CLAMP, SamplerState::CLAMP);

    mTorus->SetEffect(mSMEffect);
    mTorus->UpdateModelBound();
    mPVWMatrices.Subscribe(mTorus->worldTransform, mSMEffect->GetPVWMatrixConstant());
    mScene->AttachChild(mTorus);

    mTrackball.Attach(mScene);
    mScene->Update();
}

void SphereMapsWindow::UpdateConstants()
{
    Matrix4x4<float> pvMatrix = mCamera->GetProjectionViewMatrix();
    Matrix4x4<float> vMatrix = mCamera->GetViewMatrix();
    Matrix4x4<float> wMatrix = mTorus->worldTransform.GetHMatrix();
#if defined(GTE_USE_MAT_VEC)
    Matrix4x4<float> pvwMatrix = pvMatrix * wMatrix;
    Matrix4x4<float> vwMatrix = vMatrix * wMatrix;
#else
    Matrix4x4<float> pvwMatrix = wMatrix * pvMatrix;
    Matrix4x4<float> vwMatrix = wMatrix * vMatrix;
#endif
    mSMEffect->SetPVWMatrix(pvwMatrix);
    mSMEffect->SetVWMatrix(vwMatrix);
    mEngine->Update(mSMEffect->GetPVWMatrixConstant());
    mEngine->Update(mSMEffect->GetVWMatrixConstant());
    mPVWMatrices.Update();
}
