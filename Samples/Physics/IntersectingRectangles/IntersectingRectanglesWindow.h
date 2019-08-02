// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2019/05/02)

#pragma once

#include <Applications/GteWindow2.h>
#include <LowLevel/GteTimer.h>
#include <Physics/GteRectangleManager.h>
#include <random>
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
