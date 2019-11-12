cbuffer Transform
{
    float4x4 WVP;
};

struct VSInput
{
    float3 position   : POSITION;
    float2 texCoord   : TEXCOORD;
    uint   texIndex   : TEXINDEX;
    float4 brightness : BRIGHTNESS;
};

struct VSOutput
{
    float4 position   : SV_POSITION;
    float2 texCoord   : TEXCOORD;
    float4 brightness : BRIGHTNESS;
	
    nointerpolation uint texIndex : TEXINDEX;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput)0;
    output.position = mul(float4(input.position, 1), WVP);
    output.texCoord = input.texCoord;
    output.texIndex = input.texIndex;
    output.brightness = input.brightness;
    return output;
}
