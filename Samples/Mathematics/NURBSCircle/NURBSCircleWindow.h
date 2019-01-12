// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.18.0 (2018/10/22)

#pragma once

#include <GTEngine.h>
using namespace gte;

class NURBSCircleWindow : public Window2
{
public:
    NURBSCircleWindow(Parameters& parameters);

    virtual void OnDisplay() override;

private:
    void DrawCurve(NURBSCurve<2, float> const* curve, float maxAngle,
        int iXCenter, int iYCenter, int iRadius);

    NURBSQuarterCircleDegree2<float> mQuarterCircleDegree2;
    NURBSQuarterCircleDegree4<float> mQuarterCircleDegree4;
    NURBSHalfCircleDegree3<float> mHalfCircleDegree3;
    NURBSFullCircleDegree3<float> mFullCircleDegree3;
};
