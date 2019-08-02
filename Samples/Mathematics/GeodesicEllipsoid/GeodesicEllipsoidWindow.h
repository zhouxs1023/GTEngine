// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.23.1 (2019/05/02)

#pragma once

#include <Applications/GteWindow2.h>
#include <Mathematics/GteEllipsoidGeodesic.h>
using namespace gte;

class GeodesicEllipsoidWindow : public Window2
{
public:
    GeodesicEllipsoidWindow(Parameters& parameters);

    virtual void OnDisplay() override;
    virtual void DrawScreenOverlay() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    void ComputeTruePath();
    void ComputeApprPath(bool subdivide);
    void ComputeApprLength();
    void ParamToXY(GVector<float> const& param, int& x, int& y);
    void XYToParam(int x, int y, GVector<float>& param);

    int mSize;
    EllipsoidGeodesic<float> mGeodesic;
    GVector<float> mParam0, mParam1;
    float mXMin, mXMax, mXDelta;
    float mYMin, mYMax, mYDelta;

    int mNumTruePoints;
    std::vector<GVector<float>> mTruePoints;
    int mNumApprPoints;
    std::vector<GVector<float>> mApprPoints;

    int mCurrNumApprPoints;
    float mTrueDistance;
    float mApprDistance;
    float mApprCurvature;

    std::array<float, 4> mTextColor;
};
