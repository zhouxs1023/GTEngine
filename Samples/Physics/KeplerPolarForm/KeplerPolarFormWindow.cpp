// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "KeplerPolarFormWindow.h"

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

    Window::Parameters parameters(L"KeplerPolarFormWindow", 0, 0, 512, 512);
    auto window = TheWindowSystem.Create<KeplerPolarFormWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.NO_IDLE_LOOP);
    TheWindowSystem.Destroy<KeplerPolarFormWindow>(window);
    return 0;
}

KeplerPolarFormWindow::KeplerPolarFormWindow(Parameters& parameters)
    :
    Window2(parameters),
    mSize(mXSize)
{
    // Set up the physics module.
    mModule.gravity = 10.0f;
    mModule.mass = 1.0f;

    float time = 0.0f;
    float deltaTime = 0.01f;
    float theta = static_cast<float>(GTE_C_QUARTER_PI);
    float thetaDot = 0.1f;
    float radius = 10.0f;
    float radiusDot = 0.1f;
    mModule.Initialize(time, deltaTime, theta, thetaDot, radius, radiusDot);

    int const imax = static_cast<int>(mModule.GetPeriod() / deltaTime);
    mPositions.resize(imax);
    for (int i = 0; i < imax; ++i)
    {
        float x = 0.5f * mSize + 10.0f * radius * cos(theta);
        float y = 0.5f * mSize + 10.0f * radius * sin(theta);
        mPositions[i] = { x, y };
        mModule.Update();
        theta = mModule.GetTheta();
    }

    mDoFlip = true;
    OnDisplay();
}

void KeplerPolarFormWindow::OnDisplay()
{
    ClearScreen(0xFFFFFFFF);

    int const halfSize = static_cast<int>(0.5f * mSize);
    int const sizeM1 = static_cast<int>(mSize - 1.0f);

    // Draw the coordinate axes.
    unsigned int const gray = 0xFFC0C0C0;
    DrawLine(0, halfSize, sizeM1, halfSize, gray);
    DrawLine(halfSize, 0, halfSize, sizeM1, gray);

    // Draw a ray from the Sun's location to the initial point.
    int x = static_cast<int>(std::lrint(mPositions[1][0]));
    int y = static_cast<int>(std::lrint(mPositions[1][0]));
    DrawLine(halfSize, halfSize, x, y, gray);

    // Draw the Sun's location.  The Sun is at the origin which happens to
    // be a focal point of the ellipse.
    unsigned int const red = 0xFF0000FF;
    DrawThickPixel(halfSize, halfSize, 1, red);

    // Draw Earth's orbit.  The orbit starts in green, finishes in blue, and
    // is a blend of the two colors between.
    int const numPositions = static_cast<int>(mPositions.size());
    float const invNumPositions = 1.0f / static_cast<float>(numPositions);
    for (int i = 1; i < numPositions; ++i)
    {
        float w = static_cast<float>(i) * invNumPositions;
        float oneMinusW = 1.0f - w;
        unsigned int blue = static_cast<unsigned int>(255.0f * oneMinusW);
        unsigned int green = static_cast<unsigned int>(255.0f * w);
        x = static_cast<int>(std::lrint(mPositions[i][0]));
        y = static_cast<int>(std::lrint(mPositions[i][1]));
        unsigned int color = 0xFF000000 | (blue << 16) | (green << 8);
        SetPixel(x, y, color);
    }

    mScreenTextureNeedsUpdate = true;
    Window2::OnDisplay();
}
