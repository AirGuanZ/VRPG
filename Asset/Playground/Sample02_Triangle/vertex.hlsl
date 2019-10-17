cbuffer Transform
{
    float4x4 transform;
};

struct VSInput
{
    float2 position : POSITION;
    float3 color    : COLOR;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float3 color    : COLOR;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput)0;
    output.position = mul(float4(input.position, 0, 1), transform);
    output.color = input.color;
    return output;
}
