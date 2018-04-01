// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "BallRubberBandWindow.h"

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

    Window::Parameters parameters(L"BallRubberBandWindow", 0, 0, 256, 256);
    auto window = TheWindowSystem.Create<BallRubberBandWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.NO_IDLE_LOOP);
    TheWindowSystem.Destroy<BallRubberBandWindow>(window);
    return 0;
}

BallRubberBandWindow::BallRubberBandWindow(Parameters& parameters)
    :
    Window2(parameters)
{
    // Set up the physics module.
    mModule.springConstant = 16.0f;
    mModule.mass = 1.0f;
    mModule.Initialize(0.0f, 0.01f, { 96.0f, 96.0f }, { 64.0f, 0.0f });

    mPosition.resize(128);
    for (auto& position : mPosition)
    {
        position = mModule.GetPosition();
        mModule.Update();
    }

    OnDisplay();
}

void BallRubberBandWindow::OnDisplay()
{
    ClearScreen(0xFFFFFFFF);

    int const halfSize = mXSize / 2, sizeM1 = mXSize - 1;
    float const fHalfSize = static_cast<float>(halfSize);

    // Draw the coordinate axes.
    DrawLine(0, halfSize, sizeM1, halfSize, 0xFFC0C0C0);
    DrawLine(halfSize, 0, halfSize, sizeM1, 0xFFC0C0C0);

    // Draw the ball's path.  The orbit starts in green, finishes in blue,
    // and is a blend of the two colors between.
    int const numPositions = static_cast<int>(mPosition.size());
    float const invNumPositions = 1.0f / numPositions;
    for (int i = 0; i < numPositions - 1; ++i)
    {
        float w = i * invNumPositions, omw = 1.0f - w;
        unsigned int blue = (unsigned int)(255.0f * omw);
        unsigned int green = (unsigned int)(255.0f * w);
        unsigned int color = (green << 8) | (blue << 16) | 0xFF000000;
        int x0 = static_cast<int>(std::lrint(mPosition[i][0] + fHalfSize));
        int y0 = static_cast<int>(std::lrint(mPosition[i][1] + fHalfSize));
        int x1 = static_cast<int>(std::lrint(mPosition[i + 1][0] + fHalfSize));
        int y1 = static_cast<int>(std::lrint(mPosition[i + 1][1] + fHalfSize));
        DrawLine(x0, y0, x1, y1, color);
    }

    mScreenTextureNeedsUpdate = true;
    Window2::OnDisplay();
}
