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
    float3 normal         : NORMAL;
    float4 shadowPosition : SHADOWPOSITION;
    float2 texCoord       : TEXCOORD;
    float4 brightness     : BRIGHTNESS;
	
    nointerpolation uint texIndex : TEXINDEX;
};

SamplerState DiffuseSampler;
Texture2DArray<float4> DiffuseTexture;

#include "../Common.hlsl"

float4 main(PSInput input) : SV_TARGET
{
    float shadowFactor;
    if(dot(input.normal, sunlightDirection) < 0)
        shadowFactor = shadowScale;
    else
        shadowFactor = computeShadowFactor(input.shadowPosition, shadowScale, PCFStep);
        
    float4 texel = DiffuseTexture.Sample(DiffuseSampler, float3(input.texCoord, input.texIndex));
    float3 light = min(MAX_LIGHT, input.brightness.rgb + shadowFactor * input.brightness.a * skylight);
    float3 linear_color = texel.rgb;
    float3 linear_result = saturate(linear_color * light);
    return float4(pow(linear_result, 1 / 2.2), 1);
}
