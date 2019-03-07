// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.22.0 (2019/02/02)

#pragma once

#include <GTEngine.h>
using namespace gte;

class MorphControllersWindow : public Window3
{
public:
    MorphControllersWindow(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    bool SetEnvironment();
    void CreateScene();

    struct Vertex
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };

    std::shared_ptr<RasterizerState> mNoCullState;
    std::shared_ptr<RasterizerState> mNoCullWireState;
    std::shared_ptr<Visual> mMorphDisk;
    std::vector<std::shared_ptr<Visual>> mMorphTarget;
    std::shared_ptr<MorphController> mMorphController;
    double mApplicationTime, mApplicationDeltaTime;
    bool mDrawTargets;
};
