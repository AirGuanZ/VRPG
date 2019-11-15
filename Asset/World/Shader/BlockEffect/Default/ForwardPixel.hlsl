cbuffer PerFrame
{
    float3 skylight;
    float shadowScale;
    float3 sunlightDirection;
    float PCFStep;
};

struct PSInput
{
    float4 position       : SV_POSITION;
    float4 shadowPosition : SHADOWPOSITION;
    float4 brightness     : BRIGHTNESS;
    float3 normal         : NORMAL;
};

#include "../Common.hlsl"

float4 main(PSInput input) : SV_TARGET
{
    float shadowFactor;
    if(dot(input.normal, sunlightDirection) < 0)
        shadowFactor = shadowScale;
    else
        shadowFactor = computeShadowFactor(input.shadowPosition, shadowScale, PCFStep);
        
    float3 light = min(MAX_LIGHT, input.brightness.rgb + shadowFactor * input.brightness.a * skylight);
    float3 linear_result = saturate(light);
    return float4(pow(linear_result, 1 / 2.2), 1);
}
