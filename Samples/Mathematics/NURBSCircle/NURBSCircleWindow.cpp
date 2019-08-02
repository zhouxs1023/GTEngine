// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.18.1 (2019/05/02)

#include "NURBSCircleWindow.h"
#include <LowLevel/GteLogReporter.h>

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

    Window::Parameters parameters(L"NURBSCircleWindow", 0, 0, 512, 512);
    auto window = TheWindowSystem.Create<NURBSCircleWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.NO_IDLE_LOOP);
    TheWindowSystem.Destroy<NURBSCircleWindow>(window);
    return 0;
}

NURBSCircleWindow::NURBSCircleWindow(Parameters& parameters)
    :
    Window2(parameters)
{
    mDoFlip = true;
    OnDisplay();
}

void NURBSCircleWindow::OnDisplay()
{
    ClearScreen(0xFFFFFFFF);

    int const dx = mXSize / 4;
    int const dy = mYSize / 4;
    int const radius = mXSize / 8;

    float const halfPi = static_cast<float>(GTE_C_HALF_PI);
    float const pi = static_cast<float>(GTE_C_PI);
    float const twoPi = static_cast<float>(GTE_C_TWO_PI);
    DrawCurve(&mQuarterCircleDegree2, halfPi, dx, dy, radius);
    DrawCurve(&mQuarterCircleDegree4, halfPi, 3 * dx, dy, radius);
    DrawCurve(&mHalfCircleDegree3, pi, dx, 3 * dy, radius);
    DrawCurve(&mFullCircleDegree3, twoPi, 3 * dx, 3 * dy, radius);

    mScreenTextureNeedsUpdate = true;
    Window2::OnDisplay();
}

void NURBSCircleWindow::DrawCurve(NURBSCurve<2, float> const* curve, float maxAngle,
    int iXCenter, int iYCenter, int iRadius)
{
    Vector2<float> const center{ static_cast<float>(iXCenter), static_cast<float>(iYCenter) };
    float const radius = static_cast<float>(iRadius);
    int x0, y0, x1, y1;

    // Draw the true circle in green with thickness to allow contrast between
    // the true circle and the NURBS circle.
    int const numSamples = 1024;
    x0 = static_cast<int>(std::lrint(center[0] + radius));
    y0 = static_cast<int>(std::lrint(center[1]));
    float const divisor = static_cast<float>(numSamples - 1);
    for (int i = 1; i < numSamples; ++i)
    {
        float angle = maxAngle * static_cast<float>(i) / divisor;
        float cs = std::cos(angle);
        float sn = std::sin(angle);
        x1 = static_cast<int>(std::lrint(center[0] + radius * cs));
        y1 = static_cast<int>(std::lrint(center[1] + radius * sn));
        DrawThickLine(x0, y0, x1, y1, 1, 0xFF00FF00);
        x0 = x1;
        y0 = y1;
    }

    // Draw the NURBS circle in blue.
    Vector2<float> values[4];
    curve->Evaluate(0.0f, 0, values);
    x0 = static_cast<int>(std::lrint(center[0] + radius * values[0][0]));
    y0 = static_cast<int>(std::lrint(center[1] + radius * values[0][1]));
    for (int i = 1; i < numSamples; ++i)
    {
        float t = static_cast<float>(i) / divisor;
        curve->Evaluate(t, 0, values);
        x1 = static_cast<int>(std::lrint(center[0] + radius * values[0][0]));
        y1 = static_cast<int>(std::lrint(center[1] + radius * values[0][1]));
        DrawLine(x0, y0, x1, y1, 0xFFFF0000);
        x0 = x1;
        y0 = y1;
    }
}
