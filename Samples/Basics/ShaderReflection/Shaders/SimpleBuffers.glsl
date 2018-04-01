// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

buffer InputBuffer
{
    float inputBuffer[];
};

buffer OutputBuffer
{
    vec4 outputBuffer[];
};

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main()
{
    ivec3 t = ivec3(gl_GlobalInvocationID.xyz);
    vec4 value = inputBuffer[0] * vec4(0.25f, 0.5f, 0.75f, 1.0f);
    outputBuffer[0] = value;
}
