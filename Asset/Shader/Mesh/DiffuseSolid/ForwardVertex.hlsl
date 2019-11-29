#include "../../ForwardShadowVertex.hlsl"

cbuffer Transform
{
    float4x4 world;
    float4x4 WVP;
};

struct VSInput
{
    float3 position : POSITION;
    float3 normal   : NORMAL;
    float2 texCoord : TEXCOORD;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float3 normal   : NORMAL;
    float2 texCoord : TEXCOORD;
    
    SHADOW_VERTEX_SHADER_OUTPUT_DECL
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput)0;
    
    float4 worldPosition = mul(float4(input.position, 1), world);
    
    output.position = mul(float4(input.position, 1), WVP);
    output.normal   = mul(float4(input.normal, 0), world);
    output.texCoord = input.texCoord;
    
    SHADOW_VERTEX_SHADER_COMPUTE_IMPL(output, worldPosition)
    
    return output;
}
