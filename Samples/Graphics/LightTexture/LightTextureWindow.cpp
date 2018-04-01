// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "LightTextureWindow.h"

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

    Window::Parameters parameters(L"LightTextureWindow", 0, 0, 1024, 1024);
    auto window = TheWindowSystem.Create<LightTextureWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
    TheWindowSystem.Destroy<LightTextureWindow>(window);
    return 0;
}

LightTextureWindow::LightTextureWindow(Parameters& parameters)
    :
    Window3(parameters),
    mUseDirectional(true)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    mEngine->SetClearColor({ 0.525f, 0.741f, 0.831f, 1.0f });

    CreateScene();
    InitializeCamera(60.0f, GetAspectRatio(), 0.01f, 100.0f, 0.005f, 0.002f,
        { 0.0f, -7.0f, 1.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });
    mPVWMatrices.Update();
}

void LightTextureWindow::OnIdle()
{
    mTimer.Measure();

    mCameraRig.Move();
    UpdateConstants();

    mEngine->ClearBuffers();
    mEngine->Draw(mTerrain);

    std::array<float, 4> textColor{ 1.0f, 1.0f, 1.0f, 1.0f };
    mEngine->Draw(8, mYSize - 24, textColor, (mUseDirectional ? "Directional" : "Point"));
    mEngine->Draw(8, mYSize - 8, textColor, mTimer.GetFPS());

    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool LightTextureWindow::OnCharPress(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 's':
    case 'S':
        if (mUseDirectional)
        {
            mPVWMatrices.Unsubscribe(mTerrain->worldTransform);
            mTerrain->SetEffect(mPLTEffect);
            mPVWMatrices.Subscribe(mTerrain->worldTransform, mPLTEffect->GetPVWMatrixConstant());
            mPVWMatrices.Update();
        }
        else
        {
            mPVWMatrices.Unsubscribe(mTerrain->worldTransform);
            mTerrain->SetEffect(mDLTEffect);
            mPVWMatrices.Subscribe(mTerrain->worldTransform, mDLTEffect->GetPVWMatrixConstant());
            mPVWMatrices.Update();
        }
        mUseDirectional = !mUseDirectional;
        return true;
    }
    return Window3::OnCharPress(key, x, y);
}

bool LightTextureWindow::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");
    std::vector<std::string> inputs =
    {
        "BTHeightField.png",
        "BTStone.png"
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

void LightTextureWindow::CreateScene()
{
    mTrackball.Set(mXSize, mYSize, mCamera);

    // Create the visual effect.  The world up-direction is (0,0,1).  Choose
    // the light to point down.
    std::shared_ptr<Material> material = std::make_shared<Material>();
    material->emissive = { 0.0f, 0.0f, 0.0f, 1.0f };
    material->ambient = { 0.5f, 0.5f, 0.5f, 1.0f };
    material->diffuse = { 0.5f, 0.5f, 0.5f, 1.0f };
    material->specular = { 1.0f, 1.0f, 1.0f, 75.0f };

    std::shared_ptr<Lighting> lighting = std::make_shared<Lighting>();
    lighting->ambient = mEngine->GetClearColor();
    lighting->attenuation = { 1.0f, 0.0f, 0.0f, 1.0f };

    std::shared_ptr<LightCameraGeometry> geometry = std::make_shared<LightCameraGeometry>();
    mLightWorldPosition = { 0.0f, 0.0f, 8.0f, 1.0f };
    mLightWorldDirection = { 0.0f, 0.0f, -1.0f, 0.0f };

    std::string stoneFile = mEnvironment.GetPath("BTStone.png");
    std::shared_ptr<Texture2> stoneTexture = WICFileIO::Load(stoneFile, true);
    stoneTexture->AutogenerateMipmaps();

    mDLTEffect = std::make_shared<DirectionalLightTextureEffect>(mProgramFactory,
        mUpdater, material, lighting, geometry, stoneTexture,
        SamplerState::MIN_L_MAG_L_MIP_L, SamplerState::CLAMP, SamplerState::CLAMP);

    mPLTEffect = std::make_shared<PointLightTextureEffect>(mProgramFactory,
        mUpdater, material, lighting, geometry, stoneTexture,
        SamplerState::MIN_L_MAG_L_MIP_L, SamplerState::CLAMP, SamplerState::CLAMP);

    // Create the height field for terrain using heights from a gray-scale
    // bitmap image.
    struct Vertex
    {
        Vector3<float> position, normal;
        Vector2<float> tcoord;
    };
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_NORMAL, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);

    std::string heightFile = mEnvironment.GetPath("BTHeightField.png");
    auto heightTexture = WICFileIO::Load(heightFile, false);
    MeshFactory mf;  // Fills in Vertex.position, Vertex.tcoord.
    mf.SetVertexFormat(vformat);
    mTerrain = mf.CreateRectangle(heightTexture->GetWidth(), heightTexture->GetHeight(), 8.0f, 8.0f);
    mTrackball.Attach(mTerrain);

    // The mesh factory creates a flat height field.  Use the height-field
    // image to generate the heights and use a random number generator to
    // perturb them, just to add some noise.
    std::mt19937 mte;
    std::uniform_real_distribution<float> rnd(-1.0f, 1.0f);
    std::shared_ptr<VertexBuffer> vbuffer = mTerrain->GetVertexBuffer();
    unsigned int numVertices = vbuffer->GetNumElements();
    Vertex* vertex = vbuffer->Get<Vertex>();
    unsigned char* heights = heightTexture->Get<unsigned char>();
    for (unsigned int i = 0; i < numVertices; ++i)
    {
        float height = static_cast<float>(heights[4 * i]) / 255.0f;
        float perturb = 0.05f * rnd(mte);
        vertex[i].position[2] = 3.0f * height + perturb;
    }

    mTerrain->SetEffect(mDLTEffect);
    mTerrain->UpdateModelNormals();  // Fill in Vertex.normal.
    mPVWMatrices.Subscribe(mTerrain->worldTransform, mDLTEffect->GetPVWMatrixConstant());
}

void LightTextureWindow::UpdateConstants()
{
    Matrix4x4<float> invWMatrix = mTerrain->worldTransform.GetHInverse();
    Vector4<float> cameraWorldPosition = mCamera->GetPosition();
    std::shared_ptr<LightCameraGeometry> const& geometry = mDLTEffect->GetGeometry();
#if defined(GTE_USE_MAT_VEC)
    geometry->cameraModelPosition = invWMatrix * cameraWorldPosition;
    if (mUseDirectional)
    {
        geometry->lightModelDirection = invWMatrix * mLightWorldDirection;
        mDLTEffect->UpdateGeometryConstant();
    }
    else
    {
        geometry->lightModelPosition = invWMatrix * mLightWorldPosition;
        mPLTEffect->UpdateGeometryConstant();
    }
#else
    geometry->cameraModelPosition = cameraWorldPosition * invWMatrix;
    if (mUseDirectional)
    {
        geometry->lightModelDirection = mLightWorldDirection * invWMatrix;
        mDLTEffect->UpdateGeometryConstant();
    }
    else
    {
        geometry->lightModelPosition = mLightWorldPosition * invWMatrix;
        mPLTEffect->UpdateGeometryConstant();
    }
#endif
    mPVWMatrices.Update();
}

