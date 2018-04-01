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

struct VS_INPUT
{
    float3 modelPosition : POSITION;
    float2 modelTCoord : TEXCOORD0;
};

struct VS_OUTPUT
{
    float vertexHeight : TEXCOORD0;
    float4 clipPosition : SV_POSITION;
};

Texture2D<float> displacementTexture;
SamplerState displacementSampler;

VS_OUTPUT VSMain(VS_INPUT input)
{
    VS_OUTPUT output;

    float4 displacedPosition;
    displacedPosition.xy = input.modelPosition.xy;
    displacedPosition.z = displacementTexture.SampleLevel(
        displacementSampler, input.modelTCoord, 0);
    displacedPosition.w = 1.0f;

    output.vertexHeight = displacedPosition.z;

#if GTE_USE_MAT_VEC
    output.clipPosition = mul(pvwMatrix, displacedPosition);
#else
    output.clipPosition = mul(displacedPosition, pvwMatrix);
#endif

    return output;
}

struct PS_INPUT
{
    float vertexHeight : TEXCOORD0;
};

struct PS_OUTPUT
{
    float4 pixelColor : SV_TARGET0;
};

PS_OUTPUT PSMain(PS_INPUT input)
{
    PS_OUTPUT output;
    output.pixelColor.rgb = input.vertexHeight;
    output.pixelColor.a = 1.0f;
    return output;
}
