// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.2 (2019/04/16)

#include "BumpMapsWindow.h"
#include "SimpleBumpMapEffect.h"
#include <LowLevel/GteLogReporter.h>
#include <Graphics/GteMeshFactory.h>
#include <Graphics/GteTexture2Effect.h>

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

    Window::Parameters parameters(L"BumpMapsWindow", 0, 0, 640, 480);
    auto window = TheWindowSystem.Create<BumpMapsWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
    TheWindowSystem.Destroy<BumpMapsWindow>(window);
    return 0;
}

BumpMapsWindow::BumpMapsWindow(Parameters& parameters)
    :
    Window3(parameters),
    mUseBumpMap(true)
{
    if (!SetEnvironment() || !CreateBumpMapEffect())
    {
        parameters.created = false;
        return;
    }

    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0f, 0.01f, 0.001f,
        { 0.0f, -0.25f, -2.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f });

    CreateScene();
}

void BumpMapsWindow::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();

    if (mUseBumpMap)
    {
        mEngine->Draw(mBumpMappedTorus);
    }
    else
    {
        mEngine->Draw(mTexturedTorus);
    }

    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool BumpMapsWindow::OnCharPress(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'b':
    case 'B':
    {
        mUseBumpMap = !mUseBumpMap;
        UpdateBumpMap();
        return true;
    }
    }

    return Window3::OnCharPress(key, x, y);
}

bool BumpMapsWindow::OnMouseMotion(MouseButton button, int x, int y, unsigned int modifiers)
{
    if (Window3::OnMouseMotion(button, x, y, modifiers))
    {
        mPVWMatrices.Update();
        UpdateBumpMap();
    }
    return true;
}

bool BumpMapsWindow::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Graphics/BumpMaps/Shaders/");
    mEnvironment.Insert(path + "/Samples/Data/");
    std::vector<std::string> inputs =
    {
        "Bricks.png",
        "BricksNormal.png"
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

bool BumpMapsWindow::CreateBumpMapEffect()
{
    bool created = false;
    mBumpMapEffect = std::make_shared<SimpleBumpMapEffect>(mProgramFactory,
        mEnvironment, created);
    LogAssert(created, "Failed to create the bump map effect.");
    return created;
}

void BumpMapsWindow::CreateScene()
{
    CreateBumpMappedTorus();
    CreateTexturedTorus();

    mScene = std::make_shared<Node>();
    mScene->AttachChild(mBumpMappedTorus);
    mScene->AttachChild(mTexturedTorus);
    mTrackball.Attach(mScene);

    AxisAngle<4, float> aa(Vector4<float>::Unit(0), (float)GTE_C_QUARTER_PI);
    mBumpMappedTorus->localTransform.SetRotation(aa);
    mTexturedTorus->localTransform.SetRotation(aa);

    mTrackball.Update();
    mPVWMatrices.Update();
    UpdateBumpMap();
}

void BumpMapsWindow::CreateBumpMappedTorus()
{
    struct Vertex
    {
        Vector3<float> position;
        Vector3<float> normal;
        Vector3<float> lightDirection;
        Vector2<float> baseTCoord;
        Vector2<float> normalTCoord;
    };

    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_NORMAL, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_COLOR, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);
    vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 1);

    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    mf.SetVertexBufferUsage(Resource::DYNAMIC_UPDATE);
    mBumpMappedTorus = mf.CreateTorus(32, 32, 1.0f, 0.4f);
    auto vbuffer = mBumpMappedTorus->GetVertexBuffer();
    unsigned int const numVertices = vbuffer->GetNumElements();
    auto* vertices = vbuffer->Get<Vertex>();
    for (unsigned int i = 0; i < numVertices; ++i)
    {
        vertices[i].baseTCoord *= 4.0f;
        if (mUseBumpMap)
        {
            vertices[i].normalTCoord *= 4.0f;
        }
    }

    mBumpMappedTorus->SetEffect(mBumpMapEffect);
    mPVWMatrices.Subscribe(mBumpMappedTorus->worldTransform, mBumpMapEffect->GetPVWMatrixConstant());

    mLightDirection = Vector4<float>::Unit(2);
    SimpleBumpMapEffect::ComputeLightVectors(mBumpMappedTorus, mLightDirection);
}

void BumpMapsWindow::CreateTexturedTorus()
{
    struct Vertex
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };

    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);

    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    mf.SetVertexBufferUsage(Resource::DYNAMIC_UPDATE);
    mTexturedTorus = mf.CreateTorus(32, 32, 1.0f, 0.4f);
    auto vbuffer = mTexturedTorus->GetVertexBuffer();
    unsigned int const numVertices = vbuffer->GetNumElements();
    auto* vertices = vbuffer->Get<Vertex>();
    for (unsigned int i = 0; i < numVertices; ++i)
    {
        vertices[i].tcoord *= 4.0f;
    }

    std::string baseName = mEnvironment.GetPath("Bricks.png");
    std::shared_ptr<Texture2> baseTexture = WICFileIO::Load(baseName, true);
    baseTexture->AutogenerateMipmaps();

    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, baseTexture,
        SamplerState::MIN_L_MAG_L_MIP_L, SamplerState::WRAP, SamplerState::WRAP);

    mTexturedTorus->SetEffect(effect);
    mPVWMatrices.Subscribe(mTexturedTorus->worldTransform, effect->GetPVWMatrixConstant());
}

void BumpMapsWindow::UpdateBumpMap()
{
    if (mUseBumpMap)
    {
        // The scene graph transformations have been updated, which means the
        // tangent-space light vectors need updating.
        SimpleBumpMapEffect::ComputeLightVectors(mBumpMappedTorus, mLightDirection);
        mEngine->Update(mBumpMappedTorus->GetVertexBuffer());
    }
}
