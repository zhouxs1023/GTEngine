// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#pragma once

#include <GTEngine.h>
using namespace gte;

class MinimumAreaCircle2DWindow : public Window2
{
public:
    MinimumAreaCircle2DWindow(Parameters& parameters);

    virtual void OnDisplay() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    enum { NUM_POINTS = 256 };
    int mNumActive;
    std::vector<Vector2<float>> mVertices;
    Circle2<float> mMinimalCircle;
    MinimumAreaCircle2<float, BSRational<UIntegerAP32>> mMAC2;
};
