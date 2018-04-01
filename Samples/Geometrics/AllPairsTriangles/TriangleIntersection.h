// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#pragma once

#include <GTEngine.h>
using namespace gte;

class TriangleIntersection
{
public:
    bool operator() (Vector3<float> U[3], Vector3<float> V[3]) const;

private:
    // The first input is the plane (determined by triangle U) and the
    // second input is triangle.
    bool Intersects(Vector3<float> U[3], Vector3<float> V[3],
        Vector3<float> segment[2]) const;
};
