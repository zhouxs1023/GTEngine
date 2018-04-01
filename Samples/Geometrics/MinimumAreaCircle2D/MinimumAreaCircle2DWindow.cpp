// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "MinimumAreaCircle2DWindow.h"

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

    Window::Parameters parameters(L"MinimumAreaCircle2DWindow", 0, 0, 512, 512);
    auto window = TheWindowSystem.Create<MinimumAreaCircle2DWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.NO_IDLE_LOOP);
    TheWindowSystem.Destroy<MinimumAreaCircle2DWindow>(window);
    return 0;
}

MinimumAreaCircle2DWindow::MinimumAreaCircle2DWindow(Parameters& parameters)
    :
    Window2(parameters),
    mNumActive(2),
    mVertices(NUM_POINTS)
{
    // Randomly generated points.
    std::mt19937 mte;
    std::uniform_real_distribution<float> rnd(0.25f*mXSize, 0.75f*mYSize);
    for (auto& v : mVertices)
    {
        v = { rnd(mte), rnd(mte) };
    }

    mMAC2(mNumActive, &mVertices[0], mMinimalCircle);
}

void MinimumAreaCircle2DWindow::OnDisplay()
{
    ClearScreen(0xFFFFFFFF);

    unsigned int const gray = 0xFF808080;
    unsigned int const blue = 0xFFFF0000;
    unsigned int const red = 0xFF0000FF;

    // Draw the minimum area circle.
    int x = static_cast<int>(std::lrint(mMinimalCircle.center[0]));
    int y = static_cast<int>(std::lrint(mMinimalCircle.center[1]));
    int radius = static_cast<int>(std::lrint(mMinimalCircle.radius));
    DrawCircle(x, y, radius, gray, false);

    // Draw the support.
    int numSupport = mMAC2.GetNumSupport();
    std::array<int, 3> support = mMAC2.GetSupport();
    for (int i0 = numSupport - 1, i1 = 0; i1 <numSupport; i0 = i1++)
    {
        int x0 = static_cast<int>(std::lrint(mVertices[support[i0]][0]));
        int y0 = static_cast<int>(std::lrint(mVertices[support[i0]][1]));
        int x1 = static_cast<int>(std::lrint(mVertices[support[i1]][0]));
        int y1 = static_cast<int>(std::lrint(mVertices[support[i1]][1]));
        DrawLine(x0, y0, x1, y1, red);
    }

    // Draw the input points.
    for (int i = 0; i < mNumActive; ++i)
    {
        x = static_cast<int>(std::lrint(mVertices[i][0]));
        y = static_cast<int>(std::lrint(mVertices[i][1]));
        DrawThickPixel(x, y, 1, blue);
    }

    mScreenTextureNeedsUpdate = true;
    Window2::OnDisplay();
}

bool MinimumAreaCircle2DWindow::OnCharPress(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'n':
    case 'N':
        if (mNumActive < NUM_POINTS)
        {
            mMAC2(++mNumActive, &mVertices[0], mMinimalCircle);
            OnDisplay();
        }
        return true;
    }

    return Window::OnCharPress(key, x, y);
}
