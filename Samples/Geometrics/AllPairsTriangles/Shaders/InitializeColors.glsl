// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

buffer color0 { uint data[]; } color0SB;
buffer color1 { uint data[]; } color1SB;

layout (local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = 1) in;
void main()
{
    ivec2 dt = ivec2(gl_GlobalInvocationID.xy);
    for (int j = 0; j < 3; ++j)
    {
        color0SB.data[3*dt.x + j] = 0;
        color1SB.data[3*dt.y + j] = 1;
    }
}
