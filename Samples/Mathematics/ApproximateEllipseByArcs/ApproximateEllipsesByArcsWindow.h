// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#pragma once

#include <GTEngine.h>
using namespace gte;

class ApproximateEllipsesByArcsWindow : public Window2
{
public:
    ApproximateEllipsesByArcsWindow(Parameters& parameters);

    virtual void OnDisplay() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    // The ellipse extents 'a' and 'b' in (x/a)^2 + (y/b)^2 = 1.
    float mA, mB;

    // The arcs that approximate the ellipse.
    int mNumArcs;
    std::vector<Vector2<float>> mPoints, mCenters;
    std::vector<float> mRadii;

    // For mapping ellipse points to screen coordinates.
    float mMultiplier;
    Vector2<float> mOffset;
};
