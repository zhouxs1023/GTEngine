// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#pragma once

#include <GTEngine.h>
using namespace gte;

class Delaunay2DWindow : public Window2
{
public:
    Delaunay2DWindow(Parameters& parameters);

    virtual void OnDisplay() override;
    virtual bool OnMouseClick(MouseButton button, MouseState state,
        int x, int y, unsigned int modifiers) override;

private:
    std::vector<Vector2<float>> mVertices;
    std::vector<int> mHull;
    Delaunay2<float, BSNumber<UIntegerAP32>> mDelaunay;
    Delaunay2<float, BSNumber<UIntegerAP32>>::SearchInfo mInfo;
    int mCurrentTriX, mCurrentTriY, mCurrentIndex;
};
