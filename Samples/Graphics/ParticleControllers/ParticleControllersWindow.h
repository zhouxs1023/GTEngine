// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.22.0 (2019/02/05)

#pragma once

#include <GTEngine.h>
#include "BloodCellController.h"
using namespace gte;

class ParticleControllersWindow : public Window3
{
public:
    ParticleControllersWindow(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    void CreateScene();

    std::shared_ptr<BlendState> mBlendState;
    std::shared_ptr<DepthStencilState> mNoDepthState;
    std::shared_ptr<RasterizerState> mWireState;

    std::shared_ptr<Particles> mParticles;
    std::shared_ptr<BloodCellController> mBloodCellController;
    Timer mApplicationTimer;
};
