struct PSInput
{
    float4 position : SV_POSITION;
};

float main(PSInput input) : SV_TARGET
{
    return input.position.z;
}
