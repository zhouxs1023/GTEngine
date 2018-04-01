// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "GlossMapsWindow.h"
#include "GlossMapEffect.h"

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

    Window::Parameters parameters(L"GlossMapsWindow", 0, 0, 640, 480);
    auto window = TheWindowSystem.Create<GlossMapsWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
    TheWindowSystem.Destroy<GlossMapsWindow>(window);
    return 0;
}

GlossMapsWindow::GlossMapsWindow(Parameters& parameters)
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
    InitializeCamera(60.0f, GetAspectRatio(), 1.0f, 100.0f, 0.01f, 0.001f,
        { 0.0f, 0.0f, z }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f });

    mTrackball.Update();
    mPVWMatrices.Update();
}

void GlossMapsWindow::OnIdle()
{
    mTimer.Measure();

    mCameraRig.Move();
    UpdateConstants();

    mEngine->ClearBuffers();
    mEngine->Draw(mSquareNoGloss);
    mEngine->Draw(mSquareGloss);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool GlossMapsWindow::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");

    if (mEnvironment.GetPath("Magic.png") == "")
    {
        LogError("Cannot find file Magic.png");
        return false;
    }

    return true;
}

void GlossMapsWindow::CreateScene()
{
    mScene = std::make_shared<Node>();

    struct Vertex
    {
        Vector3<float> position, normal;
        Vector2<float> tcoord;
    };

    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_NORMAL, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);

    std::shared_ptr<VertexBuffer> vbuffer = std::make_shared<VertexBuffer>(vformat, 4);
    Vertex* vertex = vbuffer->Get<Vertex>();
    vertex[0].position = { -0.5f, 0.0f, -0.5f };
    vertex[0].normal = { 0.0f, 1.0f, 0.0f };
    vertex[0].tcoord = { 1.0f, 1.0f };
    vertex[1].position = { -0.5f, 0.0f, +0.5f };
    vertex[1].normal = { 0.0f, 1.0f, 0.0f };
    vertex[1].tcoord = { 1.0f, 0.0f };
    vertex[2].position = { +0.5f, 0.0f, +0.5f };
    vertex[2].normal = { 0.0f, 1.0f, 0.0f };
    vertex[2].tcoord = { 0.0f, 0.0f };
    vertex[3].position = { +0.5f, 0.0f, -0.5f };
    vertex[3].normal = { 0.0f, 1.0f, 0.0f };
    vertex[3].tcoord = { 0.0f, 1.0f };

    std::shared_ptr<IndexBuffer> ibuffer =
        std::make_shared<IndexBuffer>(IP_TRIMESH, 2, sizeof(unsigned int));
    unsigned int* indices = ibuffer->Get<unsigned int>();
    indices[0] = 0;  indices[1] = 1;  indices[2] = 3;
    indices[3] = 3;  indices[4] = 1;  indices[5] = 2;

    std::shared_ptr<Material> material = std::make_shared<Material>();
    material->emissive = { 0.0f, 0.0f, 0.0f, 1.0f };
    material->ambient = { 0.2f, 0.2f, 0.2f, 1.0f };
    material->diffuse = { 0.7f, 0.7f, 0.7f, 1.0f };
    material->specular = { 1.0f, 1.0f, 1.0f, 25.0f };

    std::shared_ptr<Lighting> lighting = std::make_shared<Lighting>();
    lighting->ambient = { 0.1f, 0.1f, 0.1f, 1.0f };
    lighting->diffuse = { 0.6f, 0.6f, 0.6f, 1.0f };
    lighting->specular = { 1.0f, 1.0f, 1.0f, 1.0f };
    lighting->attenuation = { 1.0f, 0.0f, 0.0f, 1.0f };

    std::shared_ptr<LightCameraGeometry> geometry0 = std::make_shared<LightCameraGeometry>();
    std::shared_ptr<LightCameraGeometry> geometry1 = std::make_shared<LightCameraGeometry>();
    mLightWorldDirection = { 0.0f, -1.0f, 0.0f, 0.0f };

    AxisAngle<4, float> aa(Vector4<float>::Unit(0), static_cast<float>(-GTE_C_QUARTER_PI));

    // Create a non-gloss-mapped square.
    mDLEffect = std::make_shared<DirectionalLightEffect>(mProgramFactory, mUpdater, 0,
        material, lighting, geometry0);
    mSquareNoGloss = std::make_shared<Visual>(vbuffer, ibuffer, mDLEffect);
    mSquareNoGloss->localTransform.SetRotation(aa);
    mSquareNoGloss->localTransform.SetTranslation(1.0f, -1.0f, 0.0f);
    mSquareNoGloss->UpdateModelBound();
    mPVWMatrices.Subscribe(mSquareNoGloss->worldTransform, mDLEffect->GetPVWMatrixConstant());
    mScene->AttachChild(mSquareNoGloss);

    // Create a gloss-mapped square.
    std::string path = mEnvironment.GetPath("Magic.png");
    std::shared_ptr<Texture2> texture = WICFileIO::Load(path, false);
    mGMEffect = std::make_shared<GlossMapEffect>(mProgramFactory, mUpdater,
        material, lighting, geometry1, texture, SamplerState::MIN_L_MAG_L_MIP_P,
        SamplerState::CLAMP, SamplerState::CLAMP);
    mSquareGloss = std::make_shared<Visual>(vbuffer, ibuffer, mGMEffect);
    mSquareGloss->localTransform.SetRotation(aa);
    mSquareGloss->localTransform.SetTranslation(-1.0f, -1.0f, 0.0f);
    mSquareGloss->UpdateModelBound();
    mPVWMatrices.Subscribe(mSquareGloss->worldTransform, mGMEffect->GetPVWMatrixConstant());
    mScene->AttachChild(mSquareGloss);

    mTrackball.Attach(mScene);
    mScene->Update();
}

void GlossMapsWindow::UpdateConstants()
{
    Matrix4x4<float> invWMatrix0 = mSquareNoGloss->worldTransform.GetHInverse();
    Matrix4x4<float> invWMatrix1 = mSquareGloss->worldTransform.GetHInverse();
    Vector4<float> cameraWorldPosition = mCamera->GetPosition();
    std::shared_ptr<LightCameraGeometry> const& geometry0 = mDLEffect->GetGeometry();
    std::shared_ptr<LightCameraGeometry> const& geometry1 = mGMEffect->GetGeometry();
#if defined(GTE_USE_MAT_VEC)
    geometry0->cameraModelPosition = invWMatrix0 * cameraWorldPosition;
    geometry0->lightModelDirection = invWMatrix0 * mLightWorldDirection;
    geometry1->cameraModelPosition = invWMatrix1 * cameraWorldPosition;
    geometry1->lightModelDirection = invWMatrix1 * mLightWorldDirection;
#else
    geometry0->cameraModelPosition = cameraWorldPosition * invWMatrix0;
    geometry0->lightModelDirection = mLightWorldDirection * invWMatrix0;
    geometry1->cameraModelPosition = cameraWorldPosition * invWMatrix1;
    geometry1->lightModelDirection = mLightWorldDirection * invWMatrix1;
#endif
    mDLEffect->UpdateGeometryConstant();
    mGMEffect->UpdateGeometryConstant();
    mPVWMatrices.Update();
}
