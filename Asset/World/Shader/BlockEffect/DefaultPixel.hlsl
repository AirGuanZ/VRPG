cbuffer Sky
{
    float3 skylight;
};

struct PSInput
{
    float4 position   : SV_POSITION;
    float4 brightness : BRIGHTNESS;
};

float4 main(PSInput input) : SV_TARGET
{
    float3 light = max(input.brightness.rgb, input.brightness.a * skylight);
    return float4(pow(light, 1 / 2.2), 1);
}
