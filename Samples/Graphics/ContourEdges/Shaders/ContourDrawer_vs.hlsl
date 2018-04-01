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
