struct GS_OUTPUT
{
    uint2 attributes : BLENDINDICES;
    float2 vertexTCoord : TEXCOORD0;
    float4 clipPosition : SV_POSITION;
};

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
