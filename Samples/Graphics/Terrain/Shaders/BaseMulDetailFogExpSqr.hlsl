// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.22.0 (2019/01/31)

cbuffer PVWMatrix
{
    float4x4 pvwMatrix;
};

cbuffer VWMatrix
{
    float4x4 vwMatrix;
};

cbuffer FogColorDensity
{
    float4 fogColorDensity;
};

struct VS_INPUT
{
    float3 modelPosition : POSITION;
    float2 modelBaseTCoord : TEXCOORD0;
    float2 modelDetailTCoord : TEXCOORD1;
};

struct VS_OUTPUT
{
    float2 vertexBaseTCoord : TEXCOORD0;
    float2 vertexDetailTCoord : TEXCOORD1;
    float4 vertexFogInfo : TEXCOORD2;
    float4 clipPosition : SV_POSITION;
};

VS_OUTPUT VSMain(VS_INPUT input)
{
    VS_OUTPUT output;

    // Transform the position from model space to clip space.
    float4 modelPosition = float4(input.modelPosition, 1.0f);
#if GTE_USE_MAT_VEC
    float3 viewPosition = mul(vwMatrix, modelPosition).xyz;
    output.clipPosition = mul(pvwMatrix, modelPosition);
#else
    float3 viewPosition = mul(modelPosition, vwMatrix).xyz;
    output.clipPosition = mul(modelPosition, pvwMatrix);
#endif

    // Transform the position from model space to view space.  This is the
    // vector from the view-space eye position (the origin) to the view-space
    // vertex position.  The fog factor (vertexFogInfo.w) uses the
    // z-component of this vector, which is z-based depth, not range-based
    // depth.
    float fogSqrDistance = dot(viewPosition, viewPosition);
    float fogExpArg = -fogColorDensity.w * fogColorDensity.w * fogSqrDistance;
    output.vertexFogInfo.rgb = fogColorDensity.rgb;
    output.vertexFogInfo.w = exp(fogExpArg);

    // Pass through the texture coordinates.
    output.vertexBaseTCoord = input.modelBaseTCoord;
    output.vertexDetailTCoord = input.modelDetailTCoord;
    return output;
}

struct PS_INPUT
{
    float2 vertexBaseTCoord : TEXCOORD0;
    float2 vertexDetailTCoord : TEXCOORD1;
    float4 vertexFogInfo : TEXCOORD2;
};

struct PS_OUTPUT
{
    float4 pixelColor : SV_TARGET0;
};

Texture2D<float4> baseTexture;
Texture2D<float4> detailTexture;
SamplerState baseSampler;
SamplerState detailSampler;

PS_OUTPUT PSMain(PS_INPUT input)
{
    PS_OUTPUT output;

    // Sample the texture images and multiply the results.
    float3 baseColor = baseTexture.Sample(baseSampler, input.vertexBaseTCoord).xyz;
    float3 detailColor = detailTexture.Sample(detailSampler, input.vertexDetailTCoord).xyz;
    float3 product = baseColor * detailColor;

    // Combine the base*detail color with the fog color.
    output.pixelColor.rgb = lerp(input.vertexFogInfo.rgb, product, input.vertexFogInfo.w);
    output.pixelColor.a = 1.0f;
    return output;
}
