// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2019/04/17)

#pragma once

#include <Applications/GteWindow3.h>
#include <Graphics/GteDirectionalLightTextureEffect.h>
#include <Graphics/GtePointLightTextureEffect.h>
using namespace gte;

class LightTextureWindow : public Window3
{
public:
    LightTextureWindow(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    bool SetEnvironment();
    void CreateScene();
    void UpdateConstants();

    std::shared_ptr<DirectionalLightTextureEffect> mDLTEffect;
    std::shared_ptr<PointLightTextureEffect> mPLTEffect;
    Vector4<float> mLightWorldPosition;
    Vector4<float> mLightWorldDirection;
    std::shared_ptr<Visual> mTerrain;
    bool mUseDirectional;
};
