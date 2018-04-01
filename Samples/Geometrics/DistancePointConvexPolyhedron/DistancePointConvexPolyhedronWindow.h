// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.5.0 (2016/11/25)

#pragma once

#include <GTEngine.h>
using namespace gte;

class DistancePointConvexPolyhedronWindow : public Window3
{
public:
    DistancePointConvexPolyhedronWindow(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    void CreateScene();
    void Translate(int direction, float delta);
    void Rotate(int direction, float delta);
    void DoQuery();

    std::shared_ptr<RasterizerState> mWireState;
    std::shared_ptr<BlendState> mBlendState;
    std::shared_ptr<Visual> mPointMesh, mPolyhedronMesh;
    std::shared_ptr<ConstantColorEffect> mRedEffect, mBlueEffect;
    std::shared_ptr<Visual> mSegment;
    Vector3<float> mPoint;
    ConvexPolyhedron3<float> mPolyhedron;
    Vector3<float> mPolyhedronCenter;
    DCPQuery<float, Vector3<float>, ConvexPolyhedron3<float>> mQuery;
};
