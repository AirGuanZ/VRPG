#include "../../ForwardShadowVertex.hlsl"

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

    SHADOW_VERTEX_SHADER_OUTPUT_DECL
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput)0;
    float4 position = float4(input.position, 1);
    
    output.position       = mul(position, VP);
    output.normal         = input.normal;
    output.texCoord       = input.texCoord;
    output.texIndex       = input.texIndex;
    output.brightness     = input.brightness;
    
    SHADOW_VERTEX_SHADER_COMPUTE_IMPL(output, position)
    
    return output;
}
