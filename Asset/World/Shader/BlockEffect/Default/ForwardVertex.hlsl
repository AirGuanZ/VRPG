cbuffer Transform
{
    float4x4 VP;
};

struct VSInput
{
    float3 position   : POSITION;
    float4 brightness : BRIGHTNESS;
    float3 normal     : NORMAL;
};

struct VSOutput
{
    float4 position           : SV_POSITION;
    float4 brightness         : BRIGHTNESS;
    float3 normal             : NORMAL;
    
    float  clipSpaceZ           : CLIP_SPACE_Z;
    float4 nearShadowPosition   : NEAR_SHADOW_POSITION;
    float4 middleShadowPosition : MIDDLE_SHADOW_POSITION;
    float4 farShadowPosition    : FAR_SHADOW_POSITION;
};

#include "../ShadowVertex.hlsl"

VSOutput main(VSInput input)
{
    float4 position = float4(input.position, 1);
    VSOutput output = (VSOutput)0;
    
    output.position   = mul(position, VP);
    output.brightness = input.brightness;
    output.normal     = input.normal;
    
    output.clipSpaceZ           = output.position.z;
    output.nearShadowPosition   = toNearShadowSpace(position);
    output.middleShadowPosition = toMiddleShadowSpace(position);
    output.farShadowPosition    = toFarShadowSpace(position);
    
    return output;
}
