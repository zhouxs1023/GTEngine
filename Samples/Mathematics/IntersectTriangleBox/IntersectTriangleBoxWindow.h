// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.2 (2019/06/04)

#pragma once

#include <Applications/GteWindow3.h>
#include <Graphics/GteConstantColorEffect.h>
#include <Mathematics/GteIntrTriangle3OrientedBox3.h>
using namespace gte;

class IntersectTriangleBoxWindow : public Window3
{
public:
    IntersectTriangleBoxWindow(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    void CreateScene();
    void Translate(int direction, float delta);
    void Rotate(int direction, float delta);
    void DoIntersectionQuery();

    // The red effect indicates the box and triangle intersect.  The blue
    // effect indicates the box and triangle do not intersect.
    std::shared_ptr<Visual> mBoxMesh;
    std::shared_ptr<ConstantColorEffect> mRedEffect, mBlueEffect;

    // The green effect is for that part of the triangle (if any) outside
    // the box.  The gray effect is for that part of the triangle (if any)
    // inside the box.
    std::shared_ptr<Visual> mOutsideTriangleMesh, mInsideTriangleMesh;
    std::shared_ptr<ConstantColorEffect> mGreenEffect, mGrayEffect;

    // The polygons are double sided.
    std::shared_ptr<RasterizerState> mNoCullState;
    std::shared_ptr<RasterizerState> mNoCullWireState;

    // All color effects have some transparency.
    std::shared_ptr<BlendState> mBlendState;

    // The world objects and intersection queries.
    OrientedBox3<float> mBox;
    Triangle3<float> mTriangle;
    TIQuery<float, Triangle3<float>, OrientedBox3<float>> mTIQuery;
    FIQuery<float, Triangle3<float>, OrientedBox3<float>> mFIQuery;
};
