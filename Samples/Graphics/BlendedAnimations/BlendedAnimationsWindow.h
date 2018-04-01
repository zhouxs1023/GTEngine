// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#pragma once

#include <GTEngine.h>
#include "BipedManager.h"
using namespace gte;

class BlendedAnimationsWindow : public Window3
{
public:
    BlendedAnimationsWindow(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;
    virtual bool OnKeyDown(int key, int x, int y) override;
    virtual bool OnKeyUp(int key, int x, int y) override;

private:
    bool SetEnvironment();
    void CreateScene();
    void GetMeshes(std::shared_ptr<Spatial> const& object);
    void Update();

    struct Vertex
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };

    std::shared_ptr<Node> mScene;
    std::shared_ptr<Visual> mFloor;
    std::shared_ptr<RasterizerState> mWireState;
    std::vector<Visual*> mMeshes;
    std::unique_ptr<BipedManager> mManager;
    double mApplicationTime, mApplicationTimeDelta;
    bool mUpArrowPressed, mShiftPressed;
};
