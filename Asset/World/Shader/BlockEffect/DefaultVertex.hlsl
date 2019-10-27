cbuffer Transform
{
    float4x4 WVP;
};

struct VSInput
{
    float3 position   : POSITION;
    float4 brightness : BRIGHTNESS;
};

struct VSOutput
{
    float4 position   : SV_POSITION;
    float4 brightness : BRIGHTNESS;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput)0;
    output.position = mul(float4(input.position, 1), WVP);
    output.brightness = input.brightness;
    return output;
}
