#include "../../ForwardShadowVertex.hlsl"

cbuffer Transform
{
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
    float4 position           : SV_POSITION;
    float4 brightness         : BRIGHTNESS;
    float3 normal             : NORMAL;

    SHADOW_VERTEX_SHADER_OUTPUT_DECL
};

VSOutput main(VSInput input)
{
    float4 position = float4(input.position, 1);
    VSOutput output = (VSOutput)0;
    
    output.position   = mul(position, VP);
    output.brightness = input.brightness;
    output.normal     = input.normal;
    
    SHADOW_VERTEX_SHADER_COMPUTE_IMPL(output, position)
    
    return output;
}
