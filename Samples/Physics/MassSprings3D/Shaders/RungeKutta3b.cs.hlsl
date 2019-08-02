// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2019/05/02)

#include "RungeKutta.cs.hlsli"

[numthreads(NUM_X_THREADS, NUM_Y_THREADS, NUM_Z_THREADS)]
void CSMain(int3 dt : SV_DispatchThreadID)
{
    int i = dt.x + dimensions.x * (dt.y + dimensions.y * dt.z);
    if (invMass[i] > 0.0f)
    {
        pTmp[i] = position[i] + delta * pAllTmp[i].d3;
        vTmp[i] = velocity[i] + delta * vAllTmp[i].d3;
    }
    else
    {
        pTmp[i] = position[i];
        vTmp[i] = 0.0f;
    }
}
