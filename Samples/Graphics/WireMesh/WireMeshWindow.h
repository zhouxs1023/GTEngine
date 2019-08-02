// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.2 (2019/05/02)

#pragma once

#include <Applications/GteWindow3.h>
using namespace gte;

class WireMeshWindow : public Window3
{
public:
    WireMeshWindow(Parameters& parameters);

    virtual void OnIdle() override;

private:
    bool SetEnvironment();
    bool CreateScene();

    std::shared_ptr<Visual> mMesh;
};
