cbuffer PerFrame
{
    float3 skylight;
    float shadowScale;
    float3 sunlightDirection;
    float dx;
};

struct PSInput
{
    float4 position       : SV_POSITION;
    float4 shadowPosition : SHADOWPOSITION;
    float4 brightness     : BRIGHTNESS;
    float3 normal         : NORMAL;
};

SamplerComparisonState ShadowSampler;
Texture2D<float> ShadowMap;

float computeShadowFactor(float4 shadowPosition, float shadowBrightness, float3 normal)
{
    if(dot(normal, sunlightDirection) < 0)
        return shadowBrightness;
        
	if(shadowPosition.z >= 0.999)
		return 1;
        
    float u = 0.5 + 0.5 *shadowPosition.x;
    float v = 0.5 - 0.5 *shadowPosition.y;
	const float2 offsets[9] =
	{
		float2(-dx, -dx),  float2(0.0f, -dx),  float2(dx, -dx),
		float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
		float2(-dx, +dx),  float2(0.0f, +dx),  float2(dx, +dx)
	};
    
	float sum = 0;
	[unroll]
	for(int i = 0; i < 9; ++i)
        sum += ShadowMap.SampleCmpLevelZero(ShadowSampler, float2(u, v) + offsets[i], shadowPosition.z);
        
    float percent = sum / 9;
    return percent + (1 - percent) * shadowBrightness;
}

float4 main(PSInput input) : SV_TARGET
{
    float shadowFactor = computeShadowFactor(input.shadowPosition, shadowScale, input.normal);
    float3 light = input.brightness.rgb + shadowFactor * input.brightness.a * skylight;
    float3 linear_result = saturate(light);
    return float4(pow(linear_result, 1 / 2.2), 1);
}
