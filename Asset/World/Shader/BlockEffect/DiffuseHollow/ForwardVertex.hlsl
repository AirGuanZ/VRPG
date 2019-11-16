cbuffer Transform
{
    float4x4 VP;
};

struct VSInput
{
    float3 position   : POSITION;
    float3 normal     : NORMAL;
    float2 texCoord   : TEXCOORD;
    float4 brightness : BRIGHTNESS;
	
    nointerpolation uint texIndex : TEXINDEX;
};

struct VSOutput
{
    float4 position   	  : SV_POSITION;
    float3 normal         : NORMAL;
    float2 texCoord   	  : TEXCOORD;
    uint   texIndex   	  : TEXINDEX;
    float4 brightness 	  : BRIGHTNESS;
    
    float  clipSpaceZ           : CLIP_SPACE_Z;
    float4 nearShadowPosition   : NEAR_SHADOW_POSITION;
    float4 middleShadowPosition : MIDDLE_SHADOW_POSITION;
    float4 farShadowPosition    : FAR_SHADOW_POSITION;
};

#include "../ShadowVertex.hlsl"

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput)0;
    float4 position = float4(input.position, 1);
    
    output.position       = mul(position, VP);
    output.normal         = input.normal;
    output.texCoord       = input.texCoord;
    output.texIndex       = input.texIndex;
    output.brightness     = input.brightness;
    
    output.clipSpaceZ           = output.position.z;
    output.nearShadowPosition   = toNearShadowSpace(position);
    output.middleShadowPosition = toMiddleShadowSpace(position);
    output.farShadowPosition    = toFarShadowSpace(position);
    
    return output;
}
