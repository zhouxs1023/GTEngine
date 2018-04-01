// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.1.0 (2016/06/28)

#pragma once

#include <GTEngine.h>
#include "PhysicsModule.h"
using namespace gte;

//#define FREE_TOP_FIXED_TIP_SINGLE_STEP

class FreeTopFixedTipWindow : public Window3
{
public:
    FreeTopFixedTipWindow(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    bool SetEnvironment();
    void InitializeModule();
    void CreateScene();
    void CreateFloor();
    void CreateAxisVertical();
    void CreateTop();
    void CreateAxisTop();
    void PhysicsTick();
    void GraphicsTick();

    // The scene graph.
    std::shared_ptr<Node> mScene, mTopRoot;
    std::shared_ptr<RasterizerState> mWireState;
    std::vector<std::shared_ptr<Visual>> mVisuals;

    // The physics system.
    PhysicsModule mModule;
    float mMaxPhi;

    // Support for clamping the frame rate.
    Timer mMotionTimer;
    double mLastUpdateTime;
};
