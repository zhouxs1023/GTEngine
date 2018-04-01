// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.5.0 (2016/11/27)

#pragma once

#include <GTEngine.h>
using namespace gte;

class DistanceOrientedBoxesWindow : public Window3
{
public:
    DistanceOrientedBoxesWindow(Parameters& parameters);

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
    std::shared_ptr<Visual> mBox0Mesh, mBox1Mesh;
    std::shared_ptr<ConstantColorEffect> mRedEffect, mBlueEffect;
    std::shared_ptr<Visual> mSegment;
    std::shared_ptr<Visual> mPoint0;
    std::shared_ptr<Visual> mPoint1;
    OrientedBox3<float> mBox0, mBox1;
    DCPQuery<float, OrientedBox3<float>, OrientedBox3<float>> mQuery;

    OrientedBox3<float>* mActiveBox;
    std::shared_ptr<Visual> mActiveBoxMesh;
};
