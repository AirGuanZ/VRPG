struct PSInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
    nointerpolation uint texIndex : TEXINDEX;
};

SamplerState DiffuseSampler;
Texture2DArray<float4> DiffuseTexture;

float main(PSInput input) : SV_TARGET
{
    float texel_a = DiffuseTexture.Sample(DiffuseSampler, float3(input.texCoord, input.texIndex)).a;
    clip(texel_a - 0.5);
    return input.position.z;
}
