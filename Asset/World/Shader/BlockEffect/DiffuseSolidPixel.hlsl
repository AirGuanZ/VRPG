cbuffer PerFrame
{
    float3 skylight;
    float shadowScale;
};

struct PSInput
{
    float4 position       : SV_POSITION;
    float4 shadowPosition : SHADOWPOSITION;
    float2 texCoord       : TEXCOORD;
    uint   texIndex       : TEXINDEX;
    float4 brightness     : BRIGHTNESS;
};

SamplerState DiffuseSampler;
Texture2DArray<float4> DiffuseTexture;

SamplerState ShadowSampler;
Texture2D<float> ShadowMap;

float computeShadowFactor(float4 shadowPosition, float shadowBrightness)
{
	float dx = 1.0 / 4096;
    float u = 0.5 + 0.5 *shadowPosition.x;
    float v = 0.5 - 0.5 *shadowPosition.y;
	if(shadowPosition.z >= 1)
		return 1;
	const float2 offsets[9] =
	{
		float2(-dx, -dx),  float2(0.0f, -dx),  float2(dx, -dx),
		float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
		float2(-dx, +dx),  float2(0.0f, +dx),  float2(dx, +dx)
	};
	float sum = 0;
	[unroll]
	for(int i = 0; i < 9; ++i)
	{
		if(shadowPosition.z < ShadowMap.Sample(ShadowSampler, float2(u, v) + offsets[i]) + 0.0003)
			sum += 1;
		else
		    sum += shadowBrightness;
	}
	return sum / 9;
}

float4 main(PSInput input) : SV_TARGET
{
    float shadowFactor = computeShadowFactor(input.shadowPosition, shadowScale);
    float4 texel = DiffuseTexture.Sample(DiffuseSampler, float3(input.texCoord, input.texIndex));
    float3 light = input.brightness.rgb + shadowFactor * input.brightness.a * skylight;
    float3 linear_color = texel.rgb;
    float3 linear_result = saturate(linear_color * light);
    return float4(pow(linear_result, 1 / 2.2), 1);
}
