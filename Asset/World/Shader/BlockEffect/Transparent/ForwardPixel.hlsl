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
    float2 texCoord       : TEXCOORD;
    float4 brightness     : BRIGHTNESS;
	
    nointerpolation uint texIndex : TEXINDEX;
};

SamplerState TransparentSampler;
Texture2DArray<float4> TransparentTexture;

#include "../Common.hlsl"

float4 main(PSInput input) : SV_TARGET
{
    float shadowFactor = computeShadowFactor(input.shadowPosition, shadowScale, PCFStep);
        
    float4 texel = TransparentTexture.Sample(TransparentSampler, float3(input.texCoord, input.texIndex));
    float3 light = min(MAX_LIGHT, input.brightness.rgb + shadowFactor * input.brightness.a * skylight);
    float3 linear_color = texel.rgb;
    float3 linear_result = saturate(linear_color * light);
    return float4(pow(linear_result, 1 / 2.2), texel.a);
}
