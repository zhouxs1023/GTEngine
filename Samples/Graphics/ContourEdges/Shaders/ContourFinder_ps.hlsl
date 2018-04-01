struct GS_OUTPUT
{
    uint2 attributes : BLENDINDICES0;
    float depth : TEXCOORD0;
    float4 clipPosition : SV_POSITION;
};

struct PS_OUTPUT
{
    uint4 attributes : SV_TARGET0;
};

PS_OUTPUT PSMain(GS_OUTPUT input)
{
    PS_OUTPUT output;
    output.attributes = uint4(input.attributes.xy, asuint(input.depth), 0);
    return output;
}
