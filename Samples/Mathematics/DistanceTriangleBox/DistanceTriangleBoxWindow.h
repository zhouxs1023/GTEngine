// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.5.1 (2019/05/02)

#pragma once

#include <Applications/GteWindow3.h>
#include <Graphics/GteConstantColorEffect.h>
#include <Mathematics/GteDistTriangle3OrientedBox3.h>
using namespace gte;

class DistanceTriangleBoxWindow : public Window3
{
public:
    DistanceTriangleBoxWindow(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    void CreateScene();
    void Translate(int direction, float delta);
    void Rotate(int direction, float delta);
    void DoQuery();

    std::shared_ptr<RasterizerState> mNoCullState;
    std::shared_ptr<RasterizerState> mNoCullWireState;
    std::shared_ptr<BlendState> mBlendState;
    std::shared_ptr<Visual> mTriangleMesh, mBoxMesh;
    std::shared_ptr<ConstantColorEffect> mRedEffect, mBlueEffect;
    std::shared_ptr<Visual> mSegment;
    Triangle3<float> mTriangle;
    OrientedBox3<float> mBox;
    DCPQuery<float, Triangle3<float>, OrientedBox3<float>> mQuery;
};
