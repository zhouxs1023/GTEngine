// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "WireMeshWindow.h"

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

    Window::Parameters parameters(L"WireMeshWindow", 0, 0, 512, 512);
    auto window = TheWindowSystem.Create<WireMeshWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
    TheWindowSystem.Destroy<WireMeshWindow>(window);
    return 0;
}

WireMeshWindow::WireMeshWindow(Parameters& parameters)
    :
    Window3(parameters)
{
    if (!SetEnvironment() || !CreateScene())
    {
        parameters.created = false;
        return;
    }

    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0f, 0.01f, 0.001f,
        { 0.0f, 0.0f, -2.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f });
    mPVWMatrices.Update();
}

void WireMeshWindow::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();
    mEngine->Draw(mMesh);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0 }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool WireMeshWindow::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Graphics/WireMesh/Shaders/");

#if defined(GTE_DEV_OPENGL)
    if (mEnvironment.GetPath("WireMeshVS.glsl") == "")
    {
        LogError("Cannot find file WireMeshVS.glsl.");
        return false;
    }
    if (mEnvironment.GetPath("WireMeshPS.glsl") == "")
    {
        LogError("Cannot find file WireMeshPS.glsl.");
        return false;
    }
    if (mEnvironment.GetPath("WireMeshGS.glsl") == "")
    {
        LogError("Cannot find file WireMeshGS.glsl.");
        return false;
    }
#else
    if (mEnvironment.GetPath("WireMesh.hlsl") == "")
    {
        LogError("Cannot find file WireMesh.hlsl.");
        return false;
    }
#endif

    return true;
}

bool WireMeshWindow::CreateScene()
{
#if defined(GTE_DEV_OPENGL)
    std::string pathVS = mEnvironment.GetPath("WireMeshVS.glsl");
    std::string pathPS = mEnvironment.GetPath("WireMeshPS.glsl");
    std::string pathGS = mEnvironment.GetPath("WireMeshGS.glsl");
    std::shared_ptr<VisualProgram> program = mProgramFactory->CreateFromFiles(pathVS, pathPS, pathGS);
#else
    std::string path = mEnvironment.GetPath("WireMesh.hlsl");
    std::shared_ptr<VisualProgram> program = mProgramFactory->CreateFromFiles(path, path, path);
#endif
    if (!program)
    {
        return false;
    }

    std::shared_ptr<ConstantBuffer> parameters = std::make_shared<ConstantBuffer>(3 * sizeof(Vector4<float>), false);
    Vector4<float>* data = parameters->Get<Vector4<float>>();
    data[0] = { 0.0f, 0.0f, 1.0f, 1.0f };  // mesh color
    data[1] = { 0.0f, 0.0f, 0.0f, 1.0f };  // edge color
    data[2] = { static_cast<float>(mXSize), static_cast<float>(mYSize), 0.0f, 0.0f };
    program->GetVShader()->Set("WireParameters", parameters);
    program->GetPShader()->Set("WireParameters", parameters);
    program->GetGShader()->Set("WireParameters", parameters);

    std::shared_ptr<ConstantBuffer> cbuffer = std::make_shared<ConstantBuffer>(sizeof(Matrix4x4<float>), true);
    program->GetVShader()->Set("PVWMatrix", cbuffer);

    std::shared_ptr<VisualEffect> effect = std::make_shared<VisualEffect>(program);

    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    mMesh = mf.CreateSphere(16, 16, 1.0f);
    mMesh->SetEffect(effect);

    mPVWMatrices.Subscribe(mMesh->worldTransform, cbuffer);

    mTrackball.Attach(mMesh);
    mTrackball.Update();
    return true;
}
