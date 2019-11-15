#include <agz/utility/file.h>

#include <VRPG/Game/Block/BasicEffect/DefaultBlockEffect.h>
#include <VRPG/Game/Misc/ShadowMappingRasterizerState.h>

VRPG_GAME_BEGIN

DefaultBlockEffect::DefaultBlockEffect()
    : forwardShadowMap_(nullptr)
{
    InitializeForward();
    InitializeShadow();
}

const char *DefaultBlockEffect::GetName() const
{
    return "default";
}

bool DefaultBlockEffect::IsTransparent() const noexcept
{
    return false;
}

void DefaultBlockEffect::StartForward() const
{
    forwardShader_.Bind();
    forwardUniforms_.Bind();
    forwardInputLayout_.Bind();
}

void DefaultBlockEffect::EndForward() const
{
    forwardInputLayout_.Unbind();
    forwardUniforms_.Unbind();
    forwardShader_.Unbind();
}

void DefaultBlockEffect::StartShadow() const
{
    shadowShader_.Bind();
    shadowUniforms_.Bind();
    shadowInputLayout_.Bind();
    shadowRasterizerState_.Bind();
}

void DefaultBlockEffect::EndShadow() const
{
    shadowShader_.Unbind();
    shadowUniforms_.Unbind();
    shadowInputLayout_.Unbind();
    shadowRasterizerState_.Unbind();
}

std::unique_ptr<PartialSectionModelBuilder> DefaultBlockEffect::CreateModelBuilder(const Vec3i &globalSectionPosition) const
{
    return std::make_unique<Builder>(globalSectionPosition, this);
}

void DefaultBlockEffect::SetForwardRenderParams(const BlockForwardRenderParams &params) const
{
    forwardShadowMap_->SetShaderResourceView(params.shadowMapSRV.Get());
    forwardVSTransform_.SetValue({ params.shadowViewProj, params.camera->GetViewProjectionMatrix() });
    forwardPSPerFrame_.SetValue({ params.skyLight, params.shadowScale, params.sunlightDirection, params.PCFStep });
}

void DefaultBlockEffect::SetShadowRenderParams(const BlockShadowRenderParams &params) const
{
    shadowVSTransform_.SetValue({ params.shadowViewProj });
}

void DefaultBlockEffect::InitializeForward()
{
    std::string vertexShaderSource = agz::file::read_txt_file("Asset/World/Shader/BlockEffect/Default/DefaultVertex.hlsl");
    std::string pixelShaderSource = agz::file::read_txt_file("Asset/World/Shader/BlockEffect/Default/DefaultPixel.hlsl");
    forwardShader_.InitializeStage<SS_VS>(vertexShaderSource);
    forwardShader_.InitializeStage<SS_PS>(pixelShaderSource);
    if(!forwardShader_.IsAllStagesAvailable())
        throw VRPGGameException("failed to initialize default block effect shader");

    forwardUniforms_ = forwardShader_.CreateUniformManager();

    forwardInputLayout_ = InputLayoutBuilder
        ("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, offsetof(Vertex, position))
        ("BRIGHTNESS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, offsetof(Vertex, brightness))
        ("NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, offsetof(Vertex, normal))
        .Build(forwardShader_.GetVertexShaderByteCode());

    forwardVSTransform_.Initialize(true, nullptr);
    forwardUniforms_.GetConstantBufferSlot<SS_VS>("Transform")->SetBuffer(forwardVSTransform_);

    Sampler shadowSampler;
    const float shadowSamplerBorderColor[] = { 1, 1, 1, 1 };
    shadowSampler.Initialize(
        D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
        D3D11_TEXTURE_ADDRESS_BORDER,
        D3D11_TEXTURE_ADDRESS_BORDER,
        D3D11_TEXTURE_ADDRESS_BORDER,
        0, 1, D3D11_COMPARISON_LESS_EQUAL,
        shadowSamplerBorderColor);
    forwardUniforms_.GetSamplerSlot<SS_PS>("ShadowSampler")->SetSampler(shadowSampler);

    forwardPSPerFrame_.Initialize(true, nullptr);
    forwardUniforms_.GetConstantBufferSlot<SS_PS>("PerFrame")->SetBuffer(forwardPSPerFrame_);

    forwardShadowMap_ = forwardUniforms_.GetShaderResourceSlot<SS_PS>("ShadowMap");
    if(!forwardShadowMap_)
        throw VRPGGameException("shader resource slot not found in default block effect shader: ShadowMap");
}

void DefaultBlockEffect::InitializeShadow()
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

float main(PSInput input) : SV_TARGET
{
    return input.position.z;
}
)___";

    shadowShader_.InitializeStage<SS_VS>(vertexShaderSource);
    shadowShader_.InitializeStage<SS_PS>(pixelShaderSource);
    if(!shadowShader_.IsAllStagesAvailable())
        throw VRPGGameException("failed to initialize shadow shader for default block effect");

    shadowUniforms_ = shadowShader_.CreateUniformManager();

    shadowVSTransform_.Initialize(true, nullptr);
    shadowUniforms_.GetConstantBufferSlot<SS_VS>("Transform")->SetBuffer(shadowVSTransform_);

    shadowInputLayout_ = InputLayoutBuilder
        ("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, offsetof(Vertex, position))
        .Build(shadowShader_.GetVertexShaderByteCode());

    shadowRasterizerState_ = CreateRasterizerStateForShadowMapping();
}

VRPG_GAME_END
