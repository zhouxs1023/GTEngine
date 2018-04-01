// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.5.0 (2016/11/26)

#pragma once

#include <GTEngine.h>
using namespace gte;

class DistanceAlignedBoxesWindow : public Window3
{
public:
    DistanceAlignedBoxesWindow(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    void CreateScene();
    void Translate(int direction, float delta);
    void DoQuery();

    std::shared_ptr<RasterizerState> mNoCullState;
    std::shared_ptr<RasterizerState> mNoCullWireState;
    std::shared_ptr<BlendState> mBlendState;
    std::shared_ptr<Visual> mBox0Mesh, mBox1Mesh;
    std::shared_ptr<ConstantColorEffect> mRedEffect, mBlueEffect;
    std::shared_ptr<Visual> mSegment;
    std::shared_ptr<Visual> mPoint0;
    std::shared_ptr<Visual> mPoint1;
    AlignedBox3<float> mBox0, mBox1;
    DCPQuery<float, AlignedBox3<float>, AlignedBox3<float>> mQuery;
};
