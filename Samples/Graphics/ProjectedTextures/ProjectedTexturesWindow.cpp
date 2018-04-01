// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "ProjectedTexturesWindow.h"

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

    Window::Parameters parameters(L"ProjectedTexturesWindow", 0, 0, 640, 489);
    auto window = TheWindowSystem.Create<ProjectedTexturesWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
    TheWindowSystem.Destroy<ProjectedTexturesWindow>(window);
    return 0;
}

ProjectedTexturesWindow::ProjectedTexturesWindow(Parameters& parameters)
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
    float z = -2.0f * mScene->worldBound.GetRadius();
    InitializeCamera(60.0f, GetAspectRatio(), 1.0f, 1000.0f, 0.01f, 0.001f,
        { 0.0f, 0.0f, z }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f });

    mTrackball.Update();
    mPVWMatrices.Update();
}

void ProjectedTexturesWindow::OnIdle()
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

bool ProjectedTexturesWindow::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");

    if (mEnvironment.GetPath("Magician.png") == "")
    {
        LogError("Cannot find file Magician.png");
        return false;
    }

    return true;
}

void ProjectedTexturesWindow::CreateScene()
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
    mTorus = mf.CreateTorus(32, 32, 40.0f, 20.0f);

    std::shared_ptr<Material> material = std::make_shared<Material>();
    material->emissive = { 0.0f, 0.0f, 0.0f, 1.0f };
    material->ambient = { 0.5f, 0.5f, 0.5f, 1.0f };
    material->diffuse = { 0.99607f, 0.83920f, 0.67059f, 1.0f };
    material->specular = { 0.8f, 0.8f, 0.8f, 0.0f };

    std::shared_ptr<Lighting> lighting = std::make_shared<Lighting>();
    lighting->ambient = { 0.25f, 0.25f, 0.25f, 1.0f };
    lighting->diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
    lighting->specular = { 0.0f, 0.0f, 0.0f, 1.0f };
    lighting->attenuation = { 1.0f, 0.0f, 0.0f, 1.0f };

    std::shared_ptr<LightCameraGeometry> geometry = std::make_shared<LightCameraGeometry>();
    mLightWorldDirection = { 0.0f, 0.0f, 1.0f, 0.0f };

    std::string path = mEnvironment.GetPath("Magician.png");
    std::shared_ptr<Texture2> texture = WICFileIO::Load(path, false);
    mPTEffect = std::make_shared<ProjectedTextureEffect>(mProgramFactory, mUpdater,
        material, lighting, geometry, texture, SamplerState::MIN_L_MAG_L_MIP_P,
        SamplerState::CLAMP, SamplerState::CLAMP);

#if defined(GTE_DEV_OPENGL)
    mProjector = std::make_shared<Camera>(true, false);
#else
    mProjector = std::make_shared<Camera>(true, true);
#endif

    mProjector->SetFrustum(1.0f, 10.0f, -0.4125f, 0.4125f, -0.55f, 0.55f);
    Vector4<float> prjDVector{ 0.0f, 0.0f, 1.0f, 0.0f };
    Vector4<float> prjUVector{ 0.0f, 1.0f, 0.0f, 0.0f };
    Vector4<float> prjRVector = Cross(prjDVector, prjUVector);
    Vector4<float> prjPosition{ 0.0f, 0.0f, -200.0f, 1.0f };
    mProjector->SetFrame(prjPosition, prjDVector, prjUVector, prjRVector);

#if defined(GTE_USE_MAT_VEC)
    Matrix4x4<float> postProjectionMatrix{
        0.5f, 0.0f, 0.0f, 0.5f,
        0.0f, 0.5f, 0.0f, 0.5f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
#else
    Matrix4x4<float> postProjectionMatrix{
        0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 1.0f
    };
#endif
    mProjector->SetPostProjectionMatrix(postProjectionMatrix);

    mTorus->SetEffect(mPTEffect);
    mTorus->UpdateModelBound();
    mPVWMatrices.Subscribe(mTorus->worldTransform, mPTEffect->GetPVWMatrixConstant());
    mScene->AttachChild(mTorus);

    mTrackball.Attach(mScene);
    mScene->Update();
}

void ProjectedTexturesWindow::UpdateConstants()
{
    Matrix4x4<float> wMatrix = mTorus->worldTransform.GetHMatrix();
    Matrix4x4<float> projPVMatrix = mProjector->GetProjectionViewMatrix();
    Matrix4x4<float> invWMatrix = mTorus->worldTransform.GetHInverse();
    Vector4<float> cameraWorldPosition = mCamera->GetPosition();
    std::shared_ptr<LightCameraGeometry> const& geometry = mPTEffect->GetGeometry();
#if defined(GTE_USE_MAT_VEC)
    Matrix4x4<float> projPVWMatrix = projPVMatrix * wMatrix;
    geometry->cameraModelPosition = invWMatrix * cameraWorldPosition;
    geometry->lightModelDirection = invWMatrix * mLightWorldDirection;
#else
    Matrix4x4<float> projPVWMatrix = wMatrix * projPVMatrix;
    geometry->cameraModelPosition = cameraWorldPosition * invWMatrix;
    geometry->lightModelDirection = mLightWorldDirection * invWMatrix;
#endif
    mPTEffect->SetProjectorMatrix(projPVWMatrix);
    mPTEffect->UpdateProjectorMatrixConstant();
    mPTEffect->UpdateGeometryConstant();
    mPVWMatrices.Update();
}
