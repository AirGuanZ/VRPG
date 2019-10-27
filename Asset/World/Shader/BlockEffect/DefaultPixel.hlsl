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
    return float4(max(input.brightness.rgb, input.brightness.a * skylight), 1);
}
