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
