// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2019/04/17)

#pragma once

#include <Applications/GteWindow3.h>
#include "DisplacementEffect.h"
using namespace gte;

class VertexTexturesWindow : public Window3
{
public:
    VertexTexturesWindow(Parameters& parameters);

    virtual void OnIdle();

private:
    bool SetEnvironment();
    void CreateMesh();

    std::shared_ptr<Visual> mHeightMesh;
    std::shared_ptr<DisplacementEffect> mEffect;
};
