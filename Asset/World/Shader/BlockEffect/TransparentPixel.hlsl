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
    float2 texCoord       : TEXCOORD;
    float4 brightness     : BRIGHTNESS;
	
    nointerpolation uint texIndex : TEXINDEX;
};

SamplerState TransparentSampler;
Texture2DArray<float4> TransparentTexture;

SamplerComparisonState ShadowSampler;
Texture2D<float> ShadowMap;

float computeShadowFactor(float4 shadowPosition, float shadowBrightness)
{
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
	float shadowFactor = computeShadowFactor(input.shadowPosition, shadowScale);
    float4 texel = TransparentTexture.Sample(TransparentSampler, float3(input.texCoord, input.texIndex));
    float3 light = input.brightness.rgb + shadowFactor * input.brightness.a * skylight;
    float3 linear_color = texel.rgb;
    float3 linear_result = saturate(linear_color * light);
    return float4(pow(linear_result, 1 / 2.2), texel.a);
}
