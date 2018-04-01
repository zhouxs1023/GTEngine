// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#pragma once

#include <GTEngine.h>
#include "AreaLightEffect.h"
using namespace gte;

class AreaLightsWindow : public Window3
{
public:
    AreaLightsWindow(Parameters& parameters);

    virtual void OnIdle();

private:
    bool SetEnvironment();
    void CreateScene();
    void CreateSurface();
    void CreateAreaLightEffect();
    void UpdateConstants();

    std::shared_ptr<Visual> mSurface;
    std::shared_ptr<Texture2> mSurfaceTexture, mNormalTexture;
    Vector4<float> mALWorldPosition, mALWorldNormal, mALWorldAxis0, mALWorldAxis1;
    Vector4<float> mALExtent;
    std::shared_ptr<AreaLightEffect> mALEffect;
};
