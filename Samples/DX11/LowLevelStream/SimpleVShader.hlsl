// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

cbuffer PVWMatrix
{
    float4x4 pvwMatrix;
};

struct VS_INPUT0
{
    float3 modelPosition : POSITION;
};

struct VS_INPUT1
{
    float4 modelColor : COLOR0;
};

struct VS_OUTPUT
{
    float4 vertexColor : COLOR0;
    float4 clipPosition : SV_POSITION;
};

VS_OUTPUT VSMain(VS_INPUT0 input0, VS_INPUT1 input1)
{
    VS_OUTPUT output;
    output.vertexColor = input1.modelColor;
    output.clipPosition = mul(pvwMatrix, float4(input0.modelPosition, 1.0f));
    return output;
}
