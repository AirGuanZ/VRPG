cbuffer Transform
{
    float4x4 ShadowVP;
    float4x4 VP;
};

struct VSInput
{
    float3 position   : POSITION;
    float4 brightness : BRIGHTNESS;
    float3 normal     : NORMAL;
};

struct VSOutput
{
    float4 position       : SV_POSITION;
    float4 shadowPosition : SHADOWPOSITION;
    float4 brightness     : BRIGHTNESS;
    float3 normal         : NORMAL;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput)0;
    output.position       = mul(float4(input.position, 1), VP);
    output.shadowPosition = mul(float4(input.position, 1), ShadowVP);
    output.brightness     = input.brightness;
    
    output.normal = input.normal;
    
    return output;
}
