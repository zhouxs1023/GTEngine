// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "AreaLightsWindow.h"

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

    Window::Parameters parameters(L"AreaLightsWindow", 0, 0, 1024, 1024);
    auto window = TheWindowSystem.Create<AreaLightsWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
    TheWindowSystem.Destroy<AreaLightsWindow>(window);
    return 0;
}

AreaLightsWindow::AreaLightsWindow(Parameters& parameters)
    :
    Window3(parameters)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    CreateScene();
    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0f, 0.01f, 0.001f,
        { 12.0f, 0.0f, 4.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });

    mPVWMatrices.Update();
}

void AreaLightsWindow::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }
    UpdateConstants();

    mEngine->ClearBuffers();
    mEngine->Draw(mSurface);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool AreaLightsWindow::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Graphics/AreaLights/Shaders/");
    mEnvironment.Insert(path + "/Samples/Data/");
    std::vector<std::string> inputs =
    {
        "Bricks.png",
        "BricksNormal.png",
#if defined(GTE_DEV_OPENGL)
        "AreaLightVS.glsl",
        "AreaLightPS.glsl"
#else
        "AreaLight.hlsl"
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

void AreaLightsWindow::CreateScene()
{
    CreateSurface();
    CreateAreaLightEffect();

    mPVWMatrices.Subscribe(mSurface->worldTransform, mALEffect->GetPVWMatrixConstant());
    mTrackball.Attach(mSurface);
    mTrackball.Update();
}

void AreaLightsWindow::CreateSurface()
{
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);

    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    mSurface = mf.CreateRectangle(2, 2, 16.0f, 16.0f);
}

void AreaLightsWindow::CreateAreaLightEffect()
{
    std::string path = mEnvironment.GetPath("Bricks.png");
    mSurfaceTexture = WICFileIO::Load(path, true);
    mSurfaceTexture->AutogenerateMipmaps();

    path = mEnvironment.GetPath("BricksNormal.png");
    mNormalTexture = WICFileIO::Load(path, true);
    mNormalTexture->AutogenerateMipmaps();

    bool created;
    mALEffect = std::make_shared<AreaLightEffect>(mProgramFactory, mEnvironment,
        mSurfaceTexture, mNormalTexture, created);
    LogAssert(created, "Unexpected condition.");

    mSurface->SetEffect(mALEffect);

    Material& surfaceMaterial =
        *mALEffect->GetMaterialConstant()->Get<Material>();

    AreaLightEffect::Parameters& areaLight =
        *mALEffect->GetAreaLightConstant()->Get<AreaLightEffect::Parameters>();

    // Gray material with tight specular.
    surfaceMaterial.emissive = { 0.0f, 0.0f, 0.0f, 1.0f };
    surfaceMaterial.ambient = { 0.25f, 0.25f, 0.25f, 1.0f };
    surfaceMaterial.diffuse = { 0.25f, 0.25f, 0.25f, 1.0f };
    surfaceMaterial.specular = { 0.5f, 0.5f, 0.5f, 128.0f };
    mEngine->Update(mALEffect->GetMaterialConstant());

    // White area light.
    areaLight.ambient = { 1.0f, 1.0f, 1.0f, 1.0f };
    areaLight.diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
    areaLight.specular = { 1.0f, 1.0f, 1.0f, 1.0f };
    areaLight.attenuation = { 1.0f, 0.0f, 0.0f, 1.0f };

    // World-space geometric information for the rectangle of the light.
    mALWorldPosition = { 0.0f, 0.0f, 32.0f, 1.0f };
    mALWorldNormal = { 0.0f, 0.0f, -1.0f, 0.0f };
    mALWorldAxis0 = { 1.0f, 0.0f, 0.0f, 0.0f };
    mALWorldAxis1 = Cross(mALWorldNormal, mALWorldAxis0);
    mALExtent = { 1.0f, 8.0f, 0.0f, 0.0f };
    areaLight.extent = mALExtent;

    UpdateConstants();
}

void AreaLightsWindow::UpdateConstants()
{
    AreaLightEffect::Parameters& areaLight =
        *mALEffect->GetAreaLightConstant()->Get<AreaLightEffect::Parameters>();

    Vector4<float>& cameraModelPosition =
        *mALEffect->GetCameraConstant()->Get<Vector4<float>>();

    Matrix4x4<float> hinverse = mSurface->worldTransform.GetHInverse();
#if defined(GTE_USE_MAT_VEC)
    areaLight.position = hinverse * mALWorldPosition;
    areaLight.normal = hinverse * mALWorldNormal;
    areaLight.axis0 = hinverse * mALWorldAxis0;
    areaLight.axis1 = hinverse * mALWorldAxis1;
    cameraModelPosition = hinverse * mCamera->GetPosition();
#else
    areaLight.position = mALWorldPosition * hinverse;
    areaLight.normal = mALWorldNormal * hinverse;
    areaLight.axis0 = mALWorldAxis0 * hinverse;
    areaLight.axis1 = mALWorldAxis1 * hinverse;
    cameraModelPosition = mCamera->GetPosition() * hinverse;
#endif

    mEngine->Update(mALEffect->GetAreaLightConstant());
    mEngine->Update(mALEffect->GetCameraConstant());
}
