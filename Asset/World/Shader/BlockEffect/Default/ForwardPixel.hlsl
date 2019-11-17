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
    
    float  clipSpaceZ           : CLIP_SPACE_Z;
    float4 nearShadowPosition   : NEAR_SHADOW_POSITION;
    float4 middleShadowPosition : MIDDLE_SHADOW_POSITION;
    float4 farShadowPosition    : FAR_SHADOW_POSITION;
};

#include "../ShadowPixel.hlsl"

float4 main(PSInput input) : SV_TARGET
{
    float shadowFactor = computeShadowFactor(input, input.normal);
    float3 light = min(MAX_BLOCK_LIGHT, input.brightness.rgb + shadowFactor * input.brightness.a * skylight);
    float3 linear_result = saturate(light);
    return float4(pow(linear_result, 1 / 2.2), 1);
}
