// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.15.0 (2018/07/31)

#include "MovingCircleRectangleWindow.h"

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

    Window::Parameters parameters(L"MovingCircleRectangleWindow", 0, 0, 768, 768);
    auto window = TheWindowSystem.Create<MovingCircleRectangleWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
    TheWindowSystem.Destroy<MovingCircleRectangleWindow>(window);
    return 0;
}

MovingCircleRectangleWindow::MovingCircleRectangleWindow(Parameters& parameters)
    :
    Window2(parameters),
    mLeftMouseDown(false),
    mRightMouseDown(false),
    mHasIntersection(false)
{
    Vector2<double> bmin = { 0.25 * mXSize, 0.375 * mYSize };
    Vector2<double> bmax = { 0.75 * mXSize, 0.625 * mYSize };
    mBox.center = 0.5 * (bmax + bmin);
    mBox.axis[0] = { 1.0, 0.0 };
    mBox.axis[1] = { 0.0, 1.0 };
    mBox.extent = 0.5 * (bmax - bmin);
    mBoxVelocity = { 0.0, 0.0 };

    mCircle.center = { 0.9 * mXSize, 0.9 * mYSize };
    mCircle.radius = 16.0;
    mCircleVelocity = { 1.0, 0.0 };

    mContactTime = 0.0;
    mContactPoint = { 0.0, 0.0 };

    mDoFlip = true;
    DoQuery();
}

void MovingCircleRectangleWindow::OnDisplay()
{
    uint32_t const white = 0xFFFFFFFF;
    uint32_t const black = 0xFF000000;
    uint32_t const gray = 0xFFF0F0F0;
    uint32_t const red = 0xFF0000FF;
    uint32_t const green = 0xFF00FF00;
    uint32_t const blue = 0xFFFF0000;
    uint32_t const orange = 0xFF0080FF;

    int bx0 = (int)mBox.center[0];
    int bx1 = (int)mBox.center[1];

    // K = { C-e0*U0-e1*U1, C+e0*U0-e1*U1, C-e0*U0+e1*U1, C+e0*U0+e1*U1 }
    std::array<Vector2<double>, 4> K;
    mBox.GetVertices(K);

    int cx0 = (int)mCircle.center[0];
    int cy0 = (int)mCircle.center[1];
    int r = (int)mCircle.radius;

    ClearScreen(white);

    // Draw the rounded rectangle.
    for (int i = 0; i < 4; ++i)
    {
        DrawCircle((int)K[i][0], (int)K[i][1], r, gray, true);
    }

    Vector2<double> T0 = K[0] - mCircle.radius * mBox.axis[1];
    Vector2<double> T1 = K[1] - mCircle.radius * mBox.axis[1];
    DrawLine((int)T0[0], (int)T0[1], (int)T1[0], (int)T1[1], gray);
    T0 = K[2] + mCircle.radius * mBox.axis[1];
    T1 = K[3] + mCircle.radius * mBox.axis[1];
    DrawLine((int)T0[0], (int)T0[1], (int)T1[0], (int)T1[1], gray);
    T0 = K[0] - mCircle.radius * mBox.axis[0];
    T1 = K[2] - mCircle.radius * mBox.axis[0];
    DrawLine((int)T0[0], (int)T0[1], (int)T1[0], (int)T1[1], gray);
    T0 = K[1] + mCircle.radius * mBox.axis[0];
    T1 = K[3] +mCircle.radius * mBox.axis[0];
    DrawLine((int)T0[0], (int)T0[1], (int)T1[0], (int)T1[1], gray);
    DrawFloodFill4(bx0, bx1, gray, white);

    // Draw the rectangle.
    DrawLine((int)K[0][0], (int)K[0][1], (int)K[1][0], (int)K[1][1], blue);
    DrawLine((int)K[1][0], (int)K[1][1], (int)K[3][0], (int)K[3][1], blue);
    DrawLine((int)K[3][0], (int)K[3][1], (int)K[2][0], (int)K[2][1], blue);
    DrawLine((int)K[2][0], (int)K[2][1], (int)K[0][0], (int)K[0][1], blue);

    // Draw the initial circle.
    DrawCircle(cx0, cy0, r, red, false);

    // Draw velocity ray with origin at the circle center.
    int cx1 = cx0 + (int)((2 * mXSize) * mCircleVelocity[0]);
    int cy1 = cy0 + (int)((2 * mXSize) * mCircleVelocity[1]);
    DrawLine(cx0, cy0, cx1, cy1, green);

    // Draw parallel velocity rays that are tangent to the circle.
    Vector2<double> vPerp = UnitPerp(mCircleVelocity);
    Vector2<double> origin = mCircle.center + mCircle.radius * vPerp;
    cx0 = (int)origin[0];
    cy0 = (int)origin[1];
    cx1 = cx0 + (int)((2 * mXSize) * mCircleVelocity[0]);
    cy1 = cy0 + (int)((2 * mXSize) * mCircleVelocity[1]);
    DrawLine(cx0, cy0, cx1, cy1, orange);

    origin = mCircle.center - mCircle.radius * vPerp;
    cx0 = (int)origin[0];
    cy0 = (int)origin[1];
    cx1 = cx0 + (int)((2 * mXSize) * mCircleVelocity[0]);
    cy1 = cy0 + (int)((2 * mXSize) * mCircleVelocity[1]);
    DrawLine(cx0, cy0, cx1, cy1, orange);

    if (mHasIntersection)
    {
        // Draw the circle at time of contact.
        cx0 = (int)(mCircle.center[0] + mContactTime * mCircleVelocity[0]);
        cy0 = (int)(mCircle.center[1] + mContactTime * mCircleVelocity[1]);
        DrawCircle(cx0, cy0, r, black, false);

        int px = (int)mContactPoint[0];
        int py = (int)mContactPoint[1];
        DrawThickPixel(px, py, 1, black);
    }

    mScreenTextureNeedsUpdate = true;
    Window2::OnDisplay();
}

bool MovingCircleRectangleWindow::OnMouseClick(int button, int state, int x, int y, unsigned int modifiers)
{
    if (button == MOUSE_LEFT)
    {
        mLeftMouseDown = (state == MOUSE_DOWN);
        ModifyVelocity(x, mYSize - 1 - y);
        return true;
    }

    if (button == MOUSE_RIGHT)
    {
        mRightMouseDown = (state == MOUSE_DOWN);
        ModifyCircle(x, mYSize - 1 - y);
        return true;
    }

    return Window2::OnMouseClick(button, state, x, y, modifiers);
}

bool MovingCircleRectangleWindow::OnMouseMotion(int button, int x, int y, unsigned int modifiers)
{
    if (button == MOUSE_LEFT)
    {
        if (mLeftMouseDown)
        {
            ModifyVelocity(x, mYSize - 1 - y);
            return true;
        }
    }

    if (button == MOUSE_RIGHT)
    {
        if (mRightMouseDown)
        {
            ModifyCircle(x, mYSize - 1 - y);
            return true;
        }
    }

    return Window2::OnMouseMotion(button, x, y, modifiers);
}

bool MovingCircleRectangleWindow::OnCharPress(unsigned char key, int x, int y)
{
    if (key == '-' || key == '_')
    {
        ModifyRectangle(-1.0);
        return true;
    }

    if (key == '+' || key == '=')
    {
        ModifyRectangle(+1.0);
        return true;
    }

    if (key == ' ')
    {
        auto result = mQuery(mBox, mBoxVelocity, mCircle, mCircleVelocity);
        mHasIntersection = (result.intersectionType != 0);
        if (mHasIntersection)
        {
            mContactTime = result.contactTime;
            mContactPoint = result.contactPoint;
        }
        return true;
    }
    return Window2::OnCharPress(key, x, y);
}

void MovingCircleRectangleWindow::DoQuery()
{
    auto result = mQuery(mBox, mBoxVelocity, mCircle, mCircleVelocity);
    mHasIntersection = (result.intersectionType != 0);
    if (mHasIntersection)
    {
        mContactTime = result.contactTime;
        mContactPoint = result.contactPoint;
    }

    OnDisplay();
}

void MovingCircleRectangleWindow::ModifyVelocity(int x, int y)
{
    int cx = (int)mCircle.center[0];
    int cy = (int)mCircle.center[1];
    mCircleVelocity[0] = double(x - cx);
    mCircleVelocity[1] = double(y - cy);
    Normalize(mCircleVelocity);
    DoQuery();
}

void MovingCircleRectangleWindow::ModifyCircle(int x, int y)
{
    mCircle.center[0] = (double)x;
    mCircle.center[1] = (double)y;
    DoQuery();
}

void MovingCircleRectangleWindow::ModifyRectangle(double direction)
{
    // Rotate the box by one degree.
    double const angle = direction * GTE_C_DEG_TO_RAD;
    double cs = cos(angle), sn = sin(angle);
    Vector2<double> temp0 = mBox.axis[0];
    Vector2<double> temp1 = mBox.axis[1];
    mBox.axis[0] = cs * temp0 - sn * temp1;
    mBox.axis[1] = sn * temp0 + cs * temp1;
    DoQuery();
}
