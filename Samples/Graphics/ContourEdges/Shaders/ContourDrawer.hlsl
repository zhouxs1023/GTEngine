// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.6.0 (2016/12/09)

// vertex shader
struct VS_INPUT
{
    uint vertexID : SV_VERTEXID;
};

struct VS_OUTPUT
{
    uint vertexID : BLENDINDICES;
};

VS_OUTPUT VSMain(VS_INPUT input)
{
    VS_OUTPUT output;
    output.vertexID = input.vertexID;
    return output;
}

// geometry shader
cbuffer MeshID
{
    uint meshID;
};

cbuffer PVWMatrix
{
    float4x4 pvwMatrix;
};

struct Vertex
{
    float3 modelPosition;
    float2 modelTCoord;
};

StructuredBuffer<Vertex> vertices;

struct GS_OUTPUT
{
    uint2 attributes : BLENDINDICES;
    float2 vertexTCoord : TEXCOORD0;
    float4 clipPosition : SV_POSITION;
};

[maxvertexcount(3)]
void GSMain(
    uint triangleID : SV_PRIMITIVEID,
    in triangleadj VS_OUTPUT input[6],
    inout TriangleStream<GS_OUTPUT> stream)
{
    GS_OUTPUT output[3];

    [unroll]
    for (int i = 0, twoI = 0; i < 3; ++i, twoI += 2)
    {
        output[i].vertexTCoord = vertices[input[twoI].vertexID].modelTCoord;

        float4 modelPosition = float4(vertices[input[twoI].vertexID].modelPosition, 1);
#if GTE_USE_MAT_VEC
        output[i].clipPosition = mul(pvwMatrix, modelPosition);
#else
        output[i].clipPosition = mul(modelPosition, pvwMatrix);
#endif
        output[i].attributes = uint2(meshID, triangleID);
    }

    stream.Append(output[0]);
    stream.Append(output[1]);
    stream.Append(output[2]);
    stream.RestartStrip();
}

// pixel shader
Texture2D<uint4> attributesTexture;
Texture2D<float4> baseTexture;
SamplerState baseSampler;

struct PS_OUTPUT
{
    float4 color : SV_TARGET0;
};

PS_OUTPUT PSMain(GS_OUTPUT input)
{
    PS_OUTPUT output;
    int2 center = (int2)input.clipPosition.xy;
    uint4 centerAttribute = attributesTexture[center];
    if (all(centerAttribute.xy == input.attributes))
    {
        output.color = float4(1, 0, 0, 1);
    }
    else
    {
        float inputDepth = input.clipPosition.z;
        float centerDepth = 1 - saturate(asfloat(centerAttribute.z));
        if (inputDepth <= centerDepth)
        {
            output.color = baseTexture.Sample(baseSampler, input.vertexTCoord);
        }
        else
        {
            output.color = float4(0, 1, 0, 1);
        }
    }
    return output;
}
