// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

struct Voxel
{
    uint index;
    uint configuration;
};

// The buffer contains only those voxels with surface in them.
// HLSL: StructuredBuffer<Voxel> voxels;
//
buffer voxels { Voxel data[]; } voxelsSB;

out Vertex
{
    uint index;
    uint configuration;
} outVertex;

void main()
{
    Voxel voxel = voxelsSB.data[gl_VertexID];
    outVertex.index = voxel.index;
    outVertex.configuration = voxel.configuration;
}
