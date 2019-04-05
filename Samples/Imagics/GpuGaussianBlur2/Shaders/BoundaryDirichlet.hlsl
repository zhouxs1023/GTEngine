// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.23.0 (2019/03/20)

Texture2D<float> inImage;
Texture2D<float> inMask;
RWTexture2D<float> outImage;

[numthreads(NUM_X_THREADS, NUM_Y_THREADS, 1)]
void CSMain(int2 t : SV_DispatchThreadID)
{
    outImage[t] = inMask[t] * inImage[t];
}
