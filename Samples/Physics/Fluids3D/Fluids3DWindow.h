// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2019/05/02)

#pragma once

#include <Applications/GteWindow3.h>
#include <Physics/GteFluid3.h>
using namespace gte;

class Fluids3DWindow : public Window3
{
public:
    Fluids3DWindow(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    bool SetEnvironment();
    bool CreateNestedBoxes();
    void UpdateConstants();

    enum { GRID_SIZE = 128 };
    std::shared_ptr<DepthStencilState> mNoDepthState;
    std::shared_ptr<RasterizerState> mNoCullingState;
    std::shared_ptr<BlendState> mAlphaState;
    std::shared_ptr<ConstantBuffer> mPVWMatrixBuffer;
    std::vector<std::shared_ptr<Visual>> mVisible;
    Fluid3 mFluid;
};
