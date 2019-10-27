struct PSInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

Texture2D<float4> color;
SamplerState colorSampler;

float4 main(PSInput input) : SV_TARGET
{
    float3 c = color.Sample(colorSampler, input.texCoord).rgb;
    return float4(pow(c, 0.6), 1);
}
