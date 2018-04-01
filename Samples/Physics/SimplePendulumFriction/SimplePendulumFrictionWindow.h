// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.3.0 (2016/07/16)

#pragma once

#include <GTEngine.h>
#include "PhysicsModule.h"
using namespace gte;

//#define SIMPLE_PENDULUM_FRICTION_SINGLE_STEP

class SimplePendulumFrictionWindow : public Window3
{
public:
    SimplePendulumFrictionWindow(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    bool SetEnvironment();
    void InitializeModule();
    void CreateScene();
    void CreateFloor();
    void CreatePendulum();
    void PhysicsTick();
    void GraphicsTick();

    struct Vertex
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };

    // The scene graph.
    std::shared_ptr<RasterizerState> mWireState;
    std::shared_ptr<Node> mScene, mPendulum;
    std::vector<std::shared_ptr<Visual>> mVisuals;
    std::array<Vector4<float>, 2> mLightWorldDirection;

    // The physics system for the simple pendulum with friction.
    PhysicsModule mModule;
    int mMotionType;
};
