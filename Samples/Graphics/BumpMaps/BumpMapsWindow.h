// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#pragma once

#include <GTEngine.h>
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
    bool CreateScene();
    bool CreateTorus();
    void UpdateBumpMap();

    std::shared_ptr<Node> mScene;
    std::shared_ptr<Visual> mTorus;
    Vector4<float> mLightDirection;
    bool mUseBumpMap;
};
