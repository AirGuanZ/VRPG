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

Texture2DArray DiffuseTexture;
SamplerState DiffuseSampler;

float4 main(PSInput input) : SV_TARGET
{
    float3 color = DiffuseTexture.Sample(DiffuseSampler, float3(input.texCoord, input.texIndex));
    float3 light = max(input.brightness.rgb, input.brightness.a * skylight);
    float3 linear_color = pow(color, 2.2);
    float3 linear_result = linear_color * light;
    return float4(pow(linear_result * light, 1 / 2.2), 1);
}