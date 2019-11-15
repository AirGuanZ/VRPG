
// max light value
#define MAX_LIGHT 2

// about shadow mapping
SamplerComparisonState ShadowSampler;
Texture2D<float> ShadowMap;

float computeShadowFactor(float4 shadowPosition, float shadowBrightness, float pcfStep)
{
    float u = 0.5 + 0.5 *shadowPosition.x;
    float v = 0.5 - 0.5 *shadowPosition.y;
	const float2 offsets[9] =
	{
		float2(-pcfStep, -pcfStep),  float2(0.0, -pcfStep),  float2(pcfStep, -pcfStep),
		float2(-pcfStep, 0.0),       float2(0.0, 0.0),       float2(pcfStep, 0.0),
		float2(-pcfStep, +pcfStep),  float2(0.0, +pcfStep),  float2(pcfStep, +pcfStep)
	};
    
	float sum = 0;
	[unroll]
	for(int i = 0; i < 9; ++i)
        sum += ShadowMap.SampleCmpLevelZero(ShadowSampler, float2(u, v) + offsets[i], shadowPosition.z);
        
    float percent = sum / 9;
    return percent + (1 - percent) * shadowBrightness;
}
