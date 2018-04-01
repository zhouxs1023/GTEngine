// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#pragma once

#include <GTEngine.h>
using namespace gte;

// After the program launches, press the key '0' to see a constrained edge
// inserted into the triangulation.  Then press key '1', and then press
// key '2'.

class ConstrainedDelaunay2DWindow : public Window2
{
public:
    ConstrainedDelaunay2DWindow(Parameters& parameters);

    virtual void OnDisplay() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;
    virtual bool OnMouseClick(MouseButton button, MouseState state, int x, int y, unsigned int modifiers) override;

private:
    std::vector<Vector2<float>> mVertices;
    std::vector<int> mHull;
    int mCurrentTriX, mCurrentTriY, mCurrentIndex;

    // The choice of N = 5 is sufficient for the data set generated in this
    // example.  Generally, it has to be larger.
    ConstrainedDelaunay2<float, BSNumber<UIntegerFP32<5>>> mDelaunay;
    ConstrainedDelaunay2<float, BSNumber<UIntegerFP32<5>>>::SearchInfo mInfo;
};
