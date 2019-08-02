// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.25.0 (2019/04/17)

Texture1DArray myTexture1;  // 2 textures in the array
Texture2DArray myTexture2;  // 2 textures in the array
SamplerState mySampler1;
SamplerState mySampler2;

struct PS_INPUT
{
    float2 vertexTCoord : TEXCOORD0;
};

struct PS_OUTPUT
{
    float4 pixelColor0 : SV_TARGET0;
};

PS_OUTPUT PSMain(PS_INPUT input)
{
    PS_OUTPUT output;
    output.pixelColor0 = 0.0f;

    float4 tcd;

    // Sample the 1D texture array.
    tcd.xy = float2(input.vertexTCoord.x, 0);
    output.pixelColor0 += myTexture1.Sample(mySampler1, tcd.xy);
    tcd.xy = float2(input.vertexTCoord.x, 1);
    output.pixelColor0 += myTexture1.Sample(mySampler1, tcd.xy);

    // Sample the 2D texture array.
    tcd.xyz = float3(input.vertexTCoord, 0);
    output.pixelColor0 += myTexture2.Sample(mySampler2, tcd.xyz);
    tcd.xyz = float3(input.vertexTCoord, 1);
    output.pixelColor0 += myTexture2.Sample(mySampler2, tcd.xyz);

    output.pixelColor0 *= 0.25f;
    return output;
};
