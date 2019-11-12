#pragma once

#include <VRPG/Game/Block/BlockEffect.h>

VRPG_GAME_BEGIN

class ShadowMapEffect : public agz::misc::uncopyable_t
{
    struct VSTransform
    {
        Mat4 VP;
    };

    Shader<SS_VS, SS_PS> shader_;
    UniformManager<SS_VS, SS_PS> uniforms_;
    ConstantBuffer<VSTransform> vsTransform_;

    InputLayout inputLayout_;

public:

    explicit ShadowMapEffect(UINT positionOffsetInVertex);

    void StartShadow();

    void EndShadow();

    void SetShadowParames(const BlockShadowRenderParams &params);
};

inline ShadowMapEffect::ShadowMapEffect(UINT positionOffsetInVertex)
{
    const char *vertexShaderSource = R"___(
cbuffer Transform
{
    float4x4 VP;
};

struct VSInput
{
    float3 position : POSITION;
};

struct VSOutput
{
    float4 position : SV_POSITION;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput)0;
    output.position = mul(float4(input.position, 1), VP);
    return output;
}
)___";

    const char *pixelShaderSource = R"___(
struct PSInput
{
    float4 position : SV_POSITION;
};

void main(PSInput input)
{
    // do nothing
}
)___";

    shader_.InitializeStage<SS_VS>(vertexShaderSource);
    shader_.InitializeStage<SS_PS>(pixelShaderSource);
    if(!shader_.IsAllStagesAvailable())
        throw VRPGWorldException("failed to initialize shader for block shadow map");

    uniforms_ = shader_.CreateUniformManager();

    vsTransform_.Initialize(true, nullptr);
    uniforms_.GetConstantBufferSlot<SS_VS>("Transform")->SetBuffer(vsTransform_);

    inputLayout_ = InputLayoutBuilder
        ("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, positionOffsetInVertex)
        .Build(shader_.GetVertexShaderByteCode());
}

inline void ShadowMapEffect::StartShadow()
{
    shader_.Bind();
    uniforms_.Bind();
    inputLayout_.Bind();
}

inline void ShadowMapEffect::EndShadow()
{
    shader_.Unbind();
    uniforms_.Unbind();
    inputLayout_.Unbind();
}

inline void ShadowMapEffect::SetShadowParames(const BlockShadowRenderParams &params)
{
    vsTransform_.SetValue({ params.shadowViewProj });
}

VRPG_GAME_END
