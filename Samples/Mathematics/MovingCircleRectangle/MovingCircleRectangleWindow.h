// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.15.0 (2018/07/30)

#pragma once

#include <GTEngine.h>
using namespace gte;

class MovingCircleRectangleWindow : public Window2
{
public:
    MovingCircleRectangleWindow(Parameters& parameters);

    virtual void OnDisplay() override;
    virtual bool OnMouseClick(int button, int state, int x, int y, unsigned int modifiers);
    virtual bool OnMouseMotion(int button, int x, int y, unsigned int modifiers);
    virtual bool OnCharPress(unsigned char key, int x, int y);

private:
    void DoQuery();
    void ModifyVelocity(int x, int y);
    void ModifyCircle(int x, int y);
    void ModifyRectangle(double direction);

    OrientedBox2<double> mBox;
    Vector2<double> mBoxVelocity;
    Circle2<double> mCircle;
    Vector2<double> mCircleVelocity;
    double mContactTime;
    Vector2<double> mContactPoint;
    FIQuery<double, OrientedBox2<double>, Circle2<double>> mQuery;
    bool mLeftMouseDown, mRightMouseDown, mHasIntersection;
};
