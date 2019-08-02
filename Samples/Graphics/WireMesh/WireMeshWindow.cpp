// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.2 (2019/05/02)

#include "WireMeshWindow.h"
#include <LowLevel/GteLogReporter.h>
#include <Graphics/GteMeshFactory.h>

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

    std::vector<std::string> inputs =
    {
        DefaultShaderName("WireMesh.vs"),
        DefaultShaderName("WireMesh.ps"),
        DefaultShaderName("WireMesh.gs")
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

bool WireMeshWindow::CreateScene()
{
    std::string vsPath = mEnvironment.GetPath(DefaultShaderName("WireMesh.vs"));
    std::string psPath = mEnvironment.GetPath(DefaultShaderName("WireMesh.ps"));
    std::string gsPath = mEnvironment.GetPath(DefaultShaderName("WireMesh.gs"));
    auto program = mProgramFactory->CreateFromFiles(vsPath, psPath, gsPath);
    if (!program)
    {
        return false;
    }

    auto parameters = std::make_shared<ConstantBuffer>(3 * sizeof(Vector4<float>), false);
    auto* data = parameters->Get<Vector4<float>>();
    data[0] = { 0.0f, 0.0f, 1.0f, 1.0f };  // mesh color
    data[1] = { 0.0f, 0.0f, 0.0f, 1.0f };  // edge color
    data[2] = { static_cast<float>(mXSize), static_cast<float>(mYSize), 0.0f, 0.0f };
    program->GetVShader()->Set("WireParameters", parameters);
    program->GetPShader()->Set("WireParameters", parameters);
    program->GetGShader()->Set("WireParameters", parameters);

    auto cbuffer = std::make_shared<ConstantBuffer>(sizeof(Matrix4x4<float>), true);
    program->GetVShader()->Set("PVWMatrix", cbuffer);

    auto effect = std::make_shared<VisualEffect>(program);

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
