// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#pragma once

#include <GTEngine.h>
using namespace gte;

class IntersectingRectanglesWindow : public Window2
{
public:
    IntersectingRectanglesWindow(Parameters& parameters);

    virtual void OnDisplay() override;
    virtual void OnIdle() override;

private:
    void ModifyRectangles();
    void DrawRectangles();

    std::vector<AlignedBox2<float>> mRectangles;
    std::unique_ptr<RectangleManager<float>> mManager;
    float mSize;
    Timer mTimer;
    double mLastIdle;
    std::mt19937 mMTE;
    std::uniform_real_distribution<float> mPerturb;
};
