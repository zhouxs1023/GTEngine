// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#pragma once

#include <GTEngine.h>
#include "PhysicsModule.h"
using namespace gte;

//#define BALL_HILL_SINGLE_STEP

class BallHillWindow : public Window3
{
public:
    BallHillWindow(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    bool SetEnvironment();
    void InitializeModule();
    void CreateScene();
    void CreateGround();
    void CreateHill();
    void CreateBall();
    void CreatePath();
    Vector4<float> UpdateBall();
    void PhysicsTick();
    void GraphicsTick();

    struct Vertex
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };

    VertexFormat mVFormat;
    MeshFactory mMeshFactory;
    std::shared_ptr<Visual> mGround, mHill, mBall, mPath;
    PhysicsModule mModule;
};
