cbuffer Shadow
{
    float3 sunlightDirection;
    float shadowScale;
    
    float nearPCFStep;
    float middlePCFStep;
    float farPCFStep;
    float _shadowPad0;
    
    float nearHomZLimit;
    float middleHomZLimit;
    float farHomZLimit;
    float _shadowPad1;
};

// max light value
#define MAX_BLOCK_LIGHT 2

// about shadow mapping

SamplerComparisonState ShadowSampler;

Texture2D<float> NearShadowMap;
Texture2D<float> MiddleShadowMap;
Texture2D<float> FarShadowMap;

float _sampleNearShadowMap(float3 shadowPosition, float2 uv)
{
	const float2 offsets[9] =
	{
		float2(-nearPCFStep, -nearPCFStep),  float2(0.0, -nearPCFStep),  float2(nearPCFStep, -nearPCFStep),
		float2(-nearPCFStep, 0.0),           float2(0.0, 0.0),           float2(nearPCFStep, 0.0),
		float2(-nearPCFStep, +nearPCFStep),  float2(0.0, +nearPCFStep),  float2(nearPCFStep, +nearPCFStep)
	};
    
    float sum = 0;
    [unroll]
    for(int i = 0; i < 9; ++i)
        sum += NearShadowMap.SampleCmpLevelZero(ShadowSampler, uv + offsets[i], shadowPosition.z);
    
    float percent = 1.0 / 9 * sum;
    return percent + (1 - percent) * shadowScale;
}

float _sampleMiddleShadowMap(float3 shadowPosition, float2 uv)
{
	const float2 offsets[9] =
	{
		float2(-middlePCFStep, -middlePCFStep),  float2(0.0, -middlePCFStep),  float2(middlePCFStep, -middlePCFStep),
		float2(-middlePCFStep, 0.0),             float2(0.0, 0.0),             float2(middlePCFStep, 0.0),
		float2(-middlePCFStep, +middlePCFStep),  float2(0.0, +middlePCFStep),  float2(middlePCFStep, +middlePCFStep)
	};
    
    float sum = 0;
    [unroll]
    for(int i = 0; i < 9; ++i)
        sum += MiddleShadowMap.SampleCmpLevelZero(ShadowSampler, uv + offsets[i], shadowPosition.z);
    
    float percent = 1.0 / 9 * sum;
    return percent + (1 - percent) * shadowScale;
}

float _sampleFarShadowMap(float3 shadowPosition, float2 uv)
{
	const float2 offsets[9] =
	{
		float2(-farPCFStep, -farPCFStep),  float2(0.0, -farPCFStep),  float2(farPCFStep, -farPCFStep),
		float2(-farPCFStep, 0.0),          float2(0.0, 0.0),          float2(farPCFStep, 0.0),
		float2(-farPCFStep, +farPCFStep),  float2(0.0, +farPCFStep),  float2(farPCFStep, +farPCFStep)
	};
    
    float sum = 0;
    [unroll]
    for(int i = 0; i < 9; ++i)
        sum += FarShadowMap.SampleCmpLevelZero(ShadowSampler, uv + offsets[i], shadowPosition.z);
    
    float percent = 1.0 / 9 * sum;
    return percent + (1 - percent) * shadowScale;
}

float computeShadowFactor_NoNormal(PSInput input)
{
    float homZ = input.clipSpaceZ;

    // near?
    
    if(homZ < nearHomZLimit)
    {
        float2 uv = float2(0.5, -0.5) * input.nearShadowPosition.xy + 0.5;
        return _sampleNearShadowMap(input.nearShadowPosition, uv);
    }
    
    // middle?
    
    if(homZ < middleHomZLimit)
    {
        float2 uv = float2(0.5, -0.5) * input.middleShadowPosition.xy + 0.5;
        return _sampleMiddleShadowMap(input.middleShadowPosition, uv);
    }
    
    // far?
    
    if(homZ < farHomZLimit)
    {
        float2 uv = float2(0.5, -0.5) * input.farShadowPosition.xy + 0.5;
        return _sampleFarShadowMap(input.farShadowPosition, uv);
    }
    
    // out of scope
    
    return 1;
}

float computeShadowFactor(PSInput input, float3 normal)
{
    if(dot(sunlightDirection, normal) <= 0)
        return shadowScale;
    return computeShadowFactor_NoNormal(input);
}
