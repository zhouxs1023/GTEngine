// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#pragma once

#include <GTEngine.h>
#include "CubeMapEffect.h"
using namespace gte;

class CubeMapsWindow : public Window3
{
public:
    CubeMapsWindow(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    bool SetEnvironment();
    bool CreateCubeMapItems();
    void CreateScene();

    std::shared_ptr<Node> mScene;
    std::shared_ptr<Visual> mSphere;
    std::shared_ptr<TextureCube> mCubeTexture;
    std::shared_ptr<CubeMapEffect> mCubeMapEffect;
    std::shared_ptr<RasterizerState> mNoCullState;
    Culler mCuller;
};
