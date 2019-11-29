#if ENABLE_SHADOW == 1

cbuffer Shadow
{
    float4x4 NearShadowVP;
    float4x4 MiddleShadowVP;
    float4x4 FarShadowVP;
};

float4 toNearShadowSpace(float4 worldPosition)
{
    return mul(worldPosition, NearShadowVP);
}

float4 toMiddleShadowSpace(float4 worldPosition)
{
    return mul(worldPosition, MiddleShadowVP);
}

float4 toFarShadowSpace(float4 worldPosition)
{
    return mul(worldPosition, FarShadowVP);
}

#define SHADOW_VERTEX_SHADER_OUTPUT_DECL                  \
    float  clipSpaceZ           : CLIP_SPACE_Z;           \
    float4 nearShadowPosition   : NEAR_SHADOW_POSITION;   \
    float4 middleShadowPosition : MIDDLE_SHADOW_POSITION; \
    float4 farShadowPosition    : FAR_SHADOW_POSITION;

#define SHADOW_VERTEX_SHADER_COMPUTE_IMPL(OUTPUT, WORLD_POSITION)      \
    OUTPUT.clipSpaceZ           = OUTPUT.position.z;                   \
    OUTPUT.nearShadowPosition   = toNearShadowSpace(WORLD_POSITION);   \
    OUTPUT.middleShadowPosition = toMiddleShadowSpace(WORLD_POSITION); \
    OUTPUT.farShadowPosition    = toFarShadowSpace(WORLD_POSITION);

#else

#define SHADOW_VERTEX_SHADER_OUTPUT_DECL

#define SHADOW_VERTEX_SHADER_COMPUTE_IMPL(OUTPUT, WORLD_POSITION)

#endif
