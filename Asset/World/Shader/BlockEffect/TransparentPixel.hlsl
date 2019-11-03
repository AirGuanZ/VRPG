cbuffer Sky
{
    float3 skylight;
};

struct PSInput
{
    float4 position   : SV_POSITION;
    float2 texCoord   : TEXCOORD;
    uint   texIndex   : TEXINDEX;
    float4 brightness : BRIGHTNESS;
};

SamplerState TransparentSampler;
Texture2DArray<float4> TransparentTexture;

float4 main(PSInput input) : SV_TARGET
{
    float4 texel = TransparentTexture.Sample(TransparentSampler, float3(input.texCoord, input.texIndex));
    float3 light = saturate(input.brightness.rgb + input.brightness.a * skylight);
    float3 linear_color = texel.rgb;
    float3 linear_result = linear_color * light;
    return float4(pow(linear_result, 1 / 2.2), texel.a);
}
