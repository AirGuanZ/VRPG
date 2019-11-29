cbuffer Transform
{
    float4x4 VP;
};

struct VSInput
{
    float3 position : POSITION;
    float2 texCoord : TEXCOORD;
    nointerpolation uint texIndex : TEXINDEX;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
    nointerpolation uint texIndex : TEXINDEX;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput)0;
    output.position = mul(float4(input.position, 1), VP);
    output.texCoord = input.texCoord;
    output.texIndex = input.texIndex;
    return output;
}
