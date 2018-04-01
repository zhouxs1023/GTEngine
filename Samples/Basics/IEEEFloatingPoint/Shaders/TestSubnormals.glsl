// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

buffer inBuffer { REAL data[]; } inBufferSB;  // Two subnormal numbers.
buffer outBuffer { REAL data[]; } outBufferSB;  // The sum of inputs, supposed to be subnormal.

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main()
{
    outBufferSB.data[0] = inBufferSB.data[0] + inBufferSB.data[1];
}
