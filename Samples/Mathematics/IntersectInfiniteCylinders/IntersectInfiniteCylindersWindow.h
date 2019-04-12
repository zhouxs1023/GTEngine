// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.24.0 (2019/04/10)

#pragma once

#include <GTEngine.h>
using namespace gte;

// Computing the curve of intersection of two (hollow) cylinders is described
// in the document
// https://www.geometrictools.com/Documentation/IntersectionInfiniteCylinders.pdf
// TODO: Factor out the intersection code into a FIQuery object (an object to
// manage the find-intersection query).

class IntersectInfiniteCylindersWindow : public Window3
{
public:
    IntersectInfiniteCylindersWindow(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    void CreateScene();

    std::shared_ptr<RasterizerState> mWireState;
    std::shared_ptr<Visual> mCylinder0, mCylinder1;
    std::shared_ptr<Visual> mCurve0, mCurve1;
    float mC0, mW1, mW2;
    float mRadius0, mRadius1, mHeight, mAngle;
};
