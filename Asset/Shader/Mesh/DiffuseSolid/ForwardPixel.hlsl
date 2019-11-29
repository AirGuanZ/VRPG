#include "../../ForwardShadowPixel.hlsl"

cbuffer PerFrame
{
    float3 skylight;
    float _perFramePad;
};

cbuffer PerObject
{
    float4 brightness;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float3 normal   : NORMAL;
    float2 texCoord : TEXCOORD;
    
    SHADOW_PIXEL_SHADER_INPUT_DECL
};

SHADOW_PIXEL_SHADER_FUNCTION_IMPL(PSInput)

SamplerState DiffuseSampler;
Texture2D<float4> DiffuseTexture;

float4 main(PSInput input) : SV_TARGET
{
    float shadowFactor = computeShadowFactor(input, input.normal);
    float4 texel = DiffuseTexture.Sample(DiffuseSampler, input.texCoord);
    float3 light = min(MAX_BLOCK_LIGHT, brightness.rgb + shadowFactor * brightness.a * skylight);
    float3 linear_color = texel.rgb;
    float3 linear_result = saturate(linear_color * light);
    return float4(pow(linear_result, 1 / 2.2), 1);
}
