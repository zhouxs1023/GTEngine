// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#pragma once

#include <GTEngine.h>
using namespace gte;

class MultipleRenderTargetsWindow : public Window3
{
public:
    MultipleRenderTargetsWindow(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    bool SetEnvironment();
    bool CreateScene();
    void CreateOverlays();

    std::string mSourceFileVS;
    std::string mSourceFilePS;

    std::shared_ptr<DrawTarget> mDrawTarget;
    std::shared_ptr<Visual> mSquare;
    std::shared_ptr<Texture2> mLinearDepth;
    std::shared_ptr<OverlayEffect> mOverlay[7];
    unsigned int mActiveOverlay;

    // Shader source code as strings.
    static std::string const msGLSLOverlayPSSource[5];
    static std::string const msHLSLOverlayPSSource[5];
    static std::string const* msOverlayPSSource[ProgramFactory::PF_NUM_API][5];
};
