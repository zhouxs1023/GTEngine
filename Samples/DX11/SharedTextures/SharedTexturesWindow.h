// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#pragma once

#include <GTEngine.h>
using namespace gte;

class SharedTexturesWindow : public Window3
{
public:
    virtual ~SharedTexturesWindow();
    SharedTexturesWindow(Parameters& parameters);

    virtual void OnIdle() override;

private:
    std::shared_ptr<DX11Engine> mSecondEngine;
    std::shared_ptr<Texture2> mSharedTexture;
    std::shared_ptr<OverlayEffect> mOverlay;
};
