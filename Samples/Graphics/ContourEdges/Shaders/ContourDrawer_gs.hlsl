struct VS_OUTPUT
{
    uint vertexID : BLENDINDICES;
};

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
