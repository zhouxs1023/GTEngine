// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2019/05/02)

#pragma once

#include <Applications/GteWindow2.h>
#include <Physics/GteFluid2.h>
using namespace gte;

//#define SAVE_RENDERING_TO_DISK

class Fluids2DWindow : public Window2
{
public:
    Fluids2DWindow(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

    enum { GRID_SIZE = 256 };

private:
    bool SetEnvironment();
    bool CreateOverlay();

    std::shared_ptr<PixelShader> mDrawDensityShader;
    std::shared_ptr<OverlayEffect> mOverlay;
    std::shared_ptr<DepthStencilState> mNoDepthState;
    std::shared_ptr<RasterizerState> mNoCullingState;
    Fluid2 mFluid;

#if defined(SAVE_RENDERING_TO_DISK)
    std::shared_ptr<DrawTarget> mTarget;
    int mVideoFrame;
#endif
};
