cbuffer Transform
{
	float4x4 ShadowVP;
    float4x4 VP;
};

struct VSInput
{
    float3 position   : POSITION;
    float2 texCoord   : TEXCOORD;
    float4 brightness : BRIGHTNESS;
	
    nointerpolation uint texIndex : TEXINDEX;
};

struct VSOutput
{
    float4 position   	  : SV_POSITION;
	float4 shadowPosition : SHADOWPOSITION;
    float2 texCoord   	  : TEXCOORD;
    uint   texIndex   	  : TEXINDEX;
    float4 brightness 	  : BRIGHTNESS;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput)0;
    output.position       = mul(float4(input.position, 1), VP);
	output.shadowPosition = mul(float4(input.position, 1), ShadowVP);
    output.texCoord       = input.texCoord;
    output.texIndex       = input.texIndex;
    output.brightness     = input.brightness;
    return output;
}
