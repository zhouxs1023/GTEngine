// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.2 (2017/07/02)

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
    float2 vertexTCoord : TEXCOORD0;
    noperspective float perspectiveDepth : TEXCOORD1;
    float4 clipPosition : SV_POSITION;
};

VS_OUTPUT VSMain(VS_INPUT input)
{
    VS_OUTPUT output;
#if GTE_USE_MAT_VEC
    output.clipPosition = mul(pvwMatrix, float4(input.modelPosition, 1.0f));
#else
    output.clipPosition = mul(float4(input.modelPosition, 1.0f), pvwMatrix);
#endif
    output.vertexTCoord = input.modelTCoord;
    output.perspectiveDepth = output.clipPosition.z / output.clipPosition.w;
    return output;
}

cbuffer FarNearRatio
{
    float farNearRatio;
};

Texture2D<float4> baseTexture;
SamplerState baseSampler;

struct PS_INPUT
{
    float2 vertexTCoord : TEXCOORD0;
    noperspective float perspectiveDepth : TEXCOORD1;
    float4 screenPosition : SV_POSITION;
};

struct PS_OUTPUT
{
    float4 color : SV_TARGET0;
    float4 screenPosition : SV_TARGET1;
    float linearDepth : SV_DEPTH;
};

PS_OUTPUT PSMain(PS_INPUT input)
{
    PS_OUTPUT output;
    output.color = baseTexture.Sample(baseSampler, input.vertexTCoord);
    output.screenPosition = input.screenPosition;

    // For Direct3D, perspective depth d is computed from the camera
    // z-value (view direction component) and the near and far plane
    // values n and f using d = f*(1-n/z)/(f-n), where z in [n,f] and
    // d in [0,1].  The values of perspectiveDepth are the interpolated
    // d-values computed by the rasterizer.  Solve for linear depth
    // L = (z-n)/(f-n) in [0,1] to obtain L = d/(r*(1-d)+d) where r = f/n.

    float d = input.perspectiveDepth;
    output.linearDepth = d / (farNearRatio * (1.0f - d) + d);
    return output;
};
