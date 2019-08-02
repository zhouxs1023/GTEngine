// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.22.1 (2019/04/17)

#include "PointControllersWindow.h"
#include <LowLevel/GteLogReporter.h>
#include <Graphics/GteVertexColorEffect.h>

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

    Window::Parameters parameters(L"PointControllersWindow", 0, 0, 512, 512);
    auto window = TheWindowSystem.Create<PointControllersWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
    TheWindowSystem.Destroy<PointControllersWindow>(window);
    return 0;
}

PointControllersWindow::PointControllersWindow(Parameters& parameters)
    :
    Window3(parameters),
    mApplicationTime(0.0),
    mApplicationDeltaTime(0.001)
{
    CreateScene();
    InitializeCamera(60.0f, GetAspectRatio(), 1.0f, 1000.0f, 0.01f, 0.01f,
        { 4.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });
    mPVWMatrices.Update();
    mTrackball.Update();
}

void PointControllersWindow::OnIdle()
{
    mTimer.Measure();

    mPoints->Update(mApplicationTime);
    mApplicationTime += mApplicationDeltaTime;

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();
    mEngine->Draw(mPoints);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

void PointControllersWindow::CreateScene()
{
    std::default_random_engine dre;
    std::uniform_real_distribution<float> urd(-1.0f, 1.0f);

    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_COLOR, DF_R32G32B32A32_FLOAT, 0);
    unsigned int numVertices = 1024;
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, numVertices);
    vbuffer->SetUsage(Resource::DYNAMIC_UPDATE);
    auto vertices = vbuffer->Get<Vertex>();
    for (unsigned int i = 0; i < numVertices; ++i)
    {
        vertices[i].position = { urd(dre), urd(dre), urd(dre) };
        vertices[i].color = { urd(dre), urd(dre), urd(dre), 1.0f };
    }

    auto ibuffer = std::make_shared<IndexBuffer>(IP_POLYPOINT, numVertices);

    auto effect = std::make_shared<VertexColorEffect>(mProgramFactory);

    mPoints = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mPoints->worldTransform, effect->GetPVWMatrixConstant());
    mTrackball.Attach(mPoints);

    mRandomController = std::make_shared<RandomController>(mUpdater);
    mPoints->AttachController(mRandomController);
}
