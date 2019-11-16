cbuffer PerFrame
{
    float3 skylight;
    float _perFramePad;
};

struct PSInput
{
    float4 position       : SV_POSITION;
    float3 normal         : NORMAL;
    float2 texCoord       : TEXCOORD;
    float4 brightness     : BRIGHTNESS;
	
    nointerpolation uint texIndex : TEXINDEX;
    
    float  clipSpaceZ           : CLIP_SPACE_Z;
    float4 nearShadowPosition   : NEAR_SHADOW_POSITION;
    float4 middleShadowPosition : MIDDLE_SHADOW_POSITION;
    float4 farShadowPosition    : FAR_SHADOW_POSITION;
};

SamplerState DiffuseSampler;
Texture2DArray<float4> DiffuseTexture;

#include "../ShadowPixel.hlsl"

float4 main(PSInput input) : SV_TARGET
{
    float shadowFactor = computeShadowFactor(input, input.normal);
    float4 texel = DiffuseTexture.Sample(DiffuseSampler, float3(input.texCoord, input.texIndex));
    float3 light = min(MAX_LIGHT, input.brightness.rgb + shadowFactor * input.brightness.a * skylight);
    float3 linear_color = texel.rgb;
    float3 linear_result = saturate(linear_color * light);
    return float4(pow(linear_result, 1 / 2.2), 1);
}
