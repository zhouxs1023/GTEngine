// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.22.1 (2019/04/17)

#pragma once

#include <Applications/GteWindow3.h>
using namespace gte;

class IKControllersWindow : public Window3
{
public:
    IKControllersWindow(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;
    virtual bool OnMouseMotion(MouseButton button, int x, int y,
        unsigned int modifiers) override;

private:
    void CreateScene();
    std::shared_ptr<Visual> CreateCube();
    std::shared_ptr<Visual> CreateRod();
    std::shared_ptr<Visual> CreateGround();
    void UpdateRod();
    bool Transform(unsigned char key);

    struct Vertex
    {
        Vector3<float> position;
        Vector4<float> color;
    };

    std::shared_ptr<RasterizerState> mWireState;
    std::shared_ptr<Node> mScene, mIKSystem, mGoal, mJoint0, mJoint1;
    std::shared_ptr<Visual> mGround, mGoalCube, mOriginCube, mEndCube, mRod;
};
