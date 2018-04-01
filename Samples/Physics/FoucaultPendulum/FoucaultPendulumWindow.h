// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.1.0 (2016/06/25)

#pragma once

#include <GTEngine.h>
#include "PhysicsModule.h"
using namespace gte;

//#define FOUCAULT_PENDULUM_SINGLE_STEP

class FoucaultPendulumWindow : public Window3
{
public:
    FoucaultPendulumWindow(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    bool SetEnvironment();
    void InitializeModule();
    void CreateScene();
    void CreateFloor();
    void CreatePath();
    void CreatePendulum();
    void PhysicsTick();
    void GraphicsTick();

    // The vertex format for the path the pendulum tip follows.
    struct VertexPC
    {
        Vector3<float> position;
        Vector4<float> color;
    };

    // The vertex format for the meshes of the pendulum.
    struct VertexPT
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };

    // The scene graph.
    std::shared_ptr<RasterizerState> mWireState;
    std::shared_ptr<Node> mScene, mPendulum;
    std::shared_ptr<Visual> mPath;
    unsigned int mNextPoint;
    float mColorDiff;
    std::vector<std::shared_ptr<Visual>> mVisuals;
    std::array<Vector4<float>, 2> mLightWorldDirection;

    // The physics system for the Foucault pendulum.
    PhysicsModule mModule;
};
