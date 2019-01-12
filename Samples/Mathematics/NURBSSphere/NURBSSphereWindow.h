// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.18.0 (2018/10/28)

#pragma once

#include <GTEngine.h>
#include <Mathematics/GteNURBSSphere.h>
using namespace gte;

class NURBSSphereWindow : public Window3
{
public:
    NURBSSphereWindow(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    void CreateScene();
    void CreateEighthSphere();
    void CreateHalfSphere();
    void CreateFullSphere();

    std::shared_ptr<RasterizerState> mNoCullSolidState;
    std::shared_ptr<RasterizerState> mNoCullWireState;
    NURBSEighthSphereDegree4<float> mEighthSphere;
    std::shared_ptr<Visual> mEighthSphereVisual;
    NURBSHalfSphereDegree3<float> mHalfSphere;
    std::shared_ptr<Visual> mHalfSphereVisual;
    NURBSFullSphereDegree3<float> mFullSphere;
    std::shared_ptr<Visual> mFullSphereVisual;

    std::shared_ptr<Visual> mCurrentVisual;
};
