// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.25.0 (2019/04/19)

struct Voxel
{
    uint index;
    uint configuration;
};

// The buffer contains only those voxels with surface in them.
StructuredBuffer<Voxel> voxels;

struct VS_INPUT
{
    uint id : SV_VertexID;
};

struct VS_OUTPUT
{
    uint index : TEXCOORD0;
    uint configuration : TEXCOORD1;
};

VS_OUTPUT VSMain (VS_INPUT input)
{
    VS_OUTPUT output;
    Voxel voxel = voxels[input.id];
    output.index = voxel.index;
    output.configuration = voxel.configuration;
    return output;
}
