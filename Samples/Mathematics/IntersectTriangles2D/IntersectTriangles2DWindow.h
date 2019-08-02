// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.23.1 (2019/05/02)

#pragma once

#include <Applications/GteWindow2.h>
#include <Mathematics/GteIntrTriangle2Triangle2.h>
using namespace gte;

class IntersectTriangles2DWindow : public Window2
{
public:
    IntersectTriangles2DWindow(Parameters& parameters);

    virtual void OnDisplay() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;
    virtual bool OnMouseClick(int button, int state, int x, int y, unsigned int modifiers) override;
    virtual bool OnMouseMotion(int button, int x, int y, unsigned int modifiers) override;

private:
    void DrawTriangle(std::array<Vector2<float>, 3> const& vertex, uint32_t colorL, uint32_t colorD);
    void DrawIntersection();
    void DoQuery();

    Triangle2<float> mTriangle[2];
    std::vector<Vector2<float>> mIntersection;
    TIQuery<float, Triangle2<float>, Triangle2<float>> mTIQuery;
    FIQuery<float, Triangle2<float>, Triangle2<float>> mFIQuery;
    int mActive;
    bool mHasIntersection;
    bool mDoTIQuery;  // true, use mTIQuery; false, use mFIQuery
};
