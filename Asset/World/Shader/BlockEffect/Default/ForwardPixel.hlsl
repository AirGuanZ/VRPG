#include "../../ForwardShadowPixel.hlsl"

cbuffer PerFrame
{
    float3 skylight;
    float _perFramePad;
};

struct PSInput
{
    float4 position       : SV_POSITION;
    float4 brightness     : BRIGHTNESS;
    float3 normal         : NORMAL;
    
    SHADOW_PIXEL_SHADER_INPUT_DECL
};

SHADOW_PIXEL_SHADER_FUNCTION_IMPL(PSInput)

float4 main(PSInput input) : SV_TARGET
{
    float shadowFactor = computeShadowFactor(input, input.normal);
    float3 light = min(MAX_BLOCK_LIGHT, input.brightness.rgb + shadowFactor * input.brightness.a * skylight);
    float3 linear_result = saturate(light);
    return float4(pow(linear_result, 1 / 2.2), 1);
}
