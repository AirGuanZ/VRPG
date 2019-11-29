// max light value
#define MAX_BLOCK_LIGHT 2

#if ENABLE_SHADOW == 1

#define SHADOW_PIXEL_SHADER_FUNCTION_IMPL(PS_INPUT_TYPE)                                                                  \
    cbuffer Shadow                                                                                                        \
    {                                                                                                                     \
        float3 sunlightDirection;                                                                                         \
        float shadowScale;                                                                                                \
                                                                                                                          \
        float enableShadow;                                                                                               \
        float nearPCFStep;                                                                                                \
        float middlePCFStep;                                                                                              \
        float farPCFStep;                                                                                                 \
                                                                                                                          \
        float nearHomZLimit;                                                                                              \
        float middleHomZLimit;                                                                                            \
        float farHomZLimit;                                                                                               \
        float _shadowPad1;                                                                                                \
    };                                                                                                                    \
                                                                                                                          \
    SamplerComparisonState ShadowSampler;                                                                                 \
                                                                                                                          \
    Texture2D<float> NearShadowMap;                                                                                       \
    Texture2D<float> MiddleShadowMap;                                                                                     \
    Texture2D<float> FarShadowMap;                                                                                        \
                                                                                                                          \
    float _sampleNearShadowMap(float3 shadowPosition, float2 uv)                                                          \
    {                                                                                                                     \
        const float2 offsets[9] =                                                                                         \
        {                                                                                                                 \
            float2(-nearPCFStep, -nearPCFStep),  float2(0.0, -nearPCFStep),  float2(nearPCFStep, -nearPCFStep),           \
            float2(-nearPCFStep, 0.0),           float2(0.0, 0.0),           float2(nearPCFStep, 0.0),                    \
            float2(-nearPCFStep, +nearPCFStep),  float2(0.0, +nearPCFStep),  float2(nearPCFStep, +nearPCFStep)            \
        };                                                                                                                \
                                                                                                                          \
        float sum = 0;                                                                                                    \
        [unroll]                                                                                                          \
        for(int i = 0; i < 9; ++i)                                                                                        \
            sum += NearShadowMap.SampleCmpLevelZero(ShadowSampler, uv + offsets[i], shadowPosition.z);                    \
                                                                                                                          \
        float percent = 1.0 / 9 * sum;                                                                                    \
        return percent + (1 - percent) * shadowScale;                                                                     \
    }                                                                                                                     \
                                                                                                                          \
    float _sampleMiddleShadowMap(float3 shadowPosition, float2 uv)                                                        \
    {                                                                                                                     \
        const float2 offsets[9] =                                                                                         \
        {                                                                                                                 \
            float2(-middlePCFStep, -middlePCFStep),  float2(0.0, -middlePCFStep),  float2(middlePCFStep, -middlePCFStep), \
            float2(-middlePCFStep, 0.0),             float2(0.0, 0.0),             float2(middlePCFStep, 0.0),            \
            float2(-middlePCFStep, +middlePCFStep),  float2(0.0, +middlePCFStep),  float2(middlePCFStep, +middlePCFStep)  \
        };                                                                                                                \
                                                                                                                          \
        float sum = 0;                                                                                                    \
        [unroll]                                                                                                          \
        for(int i = 0; i < 9; ++i)                                                                                        \
            sum += MiddleShadowMap.SampleCmpLevelZero(ShadowSampler, uv + offsets[i], shadowPosition.z);                  \
                                                                                                                          \
        float percent = 1.0 / 9 * sum;                                                                                    \
        return percent + (1 - percent) * shadowScale;                                                                     \
    }                                                                                                                     \
                                                                                                                          \
    float _sampleFarShadowMap(float3 shadowPosition, float2 uv)                                                           \
    {                                                                                                                     \
        const float2 offsets[9] =                                                                                         \
        {                                                                                                                 \
            float2(-farPCFStep, -farPCFStep),  float2(0.0, -farPCFStep),  float2(farPCFStep, -farPCFStep),                \
            float2(-farPCFStep, 0.0),          float2(0.0, 0.0),          float2(farPCFStep, 0.0),                        \
            float2(-farPCFStep, +farPCFStep),  float2(0.0, +farPCFStep),  float2(farPCFStep, +farPCFStep)                 \
        };                                                                                                                \
                                                                                                                          \
        float sum = 0;                                                                                                    \
        [unroll]                                                                                                          \
        for(int i = 0; i < 9; ++i)                                                                                        \
            sum += FarShadowMap.SampleCmpLevelZero(ShadowSampler, uv + offsets[i], shadowPosition.z);                     \
                                                                                                                          \
        float percent = 1.0 / 9 * sum;                                                                                    \
        return percent + (1 - percent) * shadowScale;                                                                     \
    }                                                                                                                     \
                                                                                                                          \
    float computeShadowFactor_NoNormal(PS_INPUT_TYPE input)                                                               \
    {                                                                                                                     \
        float homZ = input.clipSpaceZ;                                                                                    \
                                                                                                                          \
        if(homZ < nearHomZLimit)                                                                                          \
        {                                                                                                                 \
            float2 uv = float2(0.5, -0.5) * input.nearShadowPosition.xy + 0.5;                                            \
            return _sampleNearShadowMap(input.nearShadowPosition, uv);                                                    \
        }                                                                                                                 \
                                                                                                                          \
        if(homZ < middleHomZLimit)                                                                                        \
        {                                                                                                                 \
            float2 uv = float2(0.5, -0.5) * input.middleShadowPosition.xy + 0.5;                                          \
            return _sampleMiddleShadowMap(input.middleShadowPosition, uv);                                                \
        }                                                                                                                 \
                                                                                                                          \
        if(homZ < farHomZLimit)                                                                                           \
        {                                                                                                                 \
            float2 uv = float2(0.5, -0.5) * input.farShadowPosition.xy + 0.5;                                             \
            return _sampleFarShadowMap(input.farShadowPosition, uv);                                                      \
        }                                                                                                                 \
                                                                                                                          \
        return 1;                                                                                                         \
    }                                                                                                                     \
                                                                                                                          \
    float computeShadowFactor(PS_INPUT_TYPE input, float3 normal)                                                         \
    {                                                                                                                     \
        if(dot(sunlightDirection, normal) <= 0)                                                                           \
            return shadowScale;                                                                                           \
        return computeShadowFactor_NoNormal(input);                                                                       \
    }
    
#define SHADOW_PIXEL_SHADER_INPUT_DECL                    \
    float  clipSpaceZ           : CLIP_SPACE_Z;           \
    float4 nearShadowPosition   : NEAR_SHADOW_POSITION;   \
    float4 middleShadowPosition : MIDDLE_SHADOW_POSITION; \
    float4 farShadowPosition    : FAR_SHADOW_POSITION;

#else

#define SHADOW_PIXEL_SHADER_FUNCTION_IMPL(PS_INPUT_TYPE)          \
                                                                  \
    float computeShadowFactor_NoNormal(PS_INPUT_TYPE input)       \
    {                                                             \
        return 1;                                                 \
    }                                                             \
                                                                  \
    float computeShadowFactor(PS_INPUT_TYPE input, float3 normal) \
    {                                                             \
        return 1;                                                 \
    }
    
#define SHADOW_PIXEL_SHADER_INPUT_DECL

#endif
