// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.22.1 (2019/05/02)

#pragma once

#include <Applications/GteWindow3.h>
#include <Graphics/GteConstantColorEffect.h>
#include <Mathematics/GteIntrSphere3Cone3.h>
using namespace gte;

class IntersectSphereConeWindow : public Window3
{
public:
    IntersectSphereConeWindow(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    void CreateScene();
    void Translate(int direction, float delta);
    void Rotate(int direction, float delta);
    void TestIntersection();

    std::shared_ptr<RasterizerState> mNoCullState;
    std::shared_ptr<RasterizerState> mNoCullWireState;
    std::shared_ptr<BlendState> mBlendState;
    std::shared_ptr<Visual> mConeMesh, mDiskMinMesh, mDiskMaxMesh, mSphereMesh;
    std::shared_ptr<ConstantColorEffect> mBlueEffect, mCyanEffect, mRedEffect;
    std::shared_ptr<ConstantColorEffect> mGreenEffect[2], mYellowEffect[2];
    float mAlpha;

    Sphere3<float> mSphere;
    Cone3<float> mCone;
    TIQuery<float, Sphere3<float>, Cone3<float>> mQuery;
};
