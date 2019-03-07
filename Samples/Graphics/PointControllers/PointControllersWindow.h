// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.22.0 (2019/02/03)

#pragma once

#include <GTEngine.h>
#include "RandomController.h"
using namespace gte;

class PointControllersWindow : public Window3
{
public:
    PointControllersWindow(Parameters& parameters);

    virtual void OnIdle() override;

private:
    void CreateScene();

    struct Vertex
    {
        Vector3<float> position;
        Vector4<float> color;
    };

    std::shared_ptr<Visual> mPoints;
    std::shared_ptr<RandomController> mRandomController;
    double mApplicationTime, mApplicationDeltaTime;
};
