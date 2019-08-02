// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2019/04/16)

#pragma once

#include <Applications/GteWindow3.h>
#include "SimpleBumpMapEffect.h"
using namespace gte;

class BumpMapsWindow : public Window3
{
public:
    BumpMapsWindow(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;
    virtual bool OnMouseMotion(MouseButton button, int x, int y, unsigned int modifiers) override;

private:
    bool SetEnvironment();
    bool CreateBumpMapEffect();
    void CreateScene();
    void CreateBumpMappedTorus();
    void CreateTexturedTorus();
    void UpdateBumpMap();

    std::shared_ptr<Node> mScene;
    std::shared_ptr<SimpleBumpMapEffect> mBumpMapEffect;
    std::shared_ptr<Visual> mBumpMappedTorus;
    std::shared_ptr<Visual> mTexturedTorus;
    Vector4<float> mLightDirection;
    bool mUseBumpMap;
};
