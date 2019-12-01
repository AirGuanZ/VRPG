#include <agz/utility/file.h>

#include <VRPG/Game/Config/GlobalConfig.h>
#include <VRPG/Game/World/Block/BasicEffect/DefaultBlockEffect.h>

VRPG_GAME_BEGIN

DefaultBlockEffect::DefaultBlockEffect()
{
    InitializeForward();
    InitializeShadow();

    forwardShadowMapping_ = CreateForwardshadowMapping(&forwardUniforms_);
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
    forwardShadowMapping_->StartForward();
    forwardShader_        .Bind();
    forwardUniforms_      .Bind();
    forwardInputLayout_   .Bind();
}

void DefaultBlockEffect::EndForward() const
{
    forwardInputLayout_   .Unbind();
    forwardUniforms_      .Unbind();
    forwardShader_        .Unbind();
    forwardShadowMapping_->EndForward();
}

void DefaultBlockEffect::StartShadow() const
{
    shadowShader_         .Bind();
    shadowUniforms_       .Bind();
    shadowInputLayout_    .Bind();
    shadowRasterizerState_.Bind();
}

void DefaultBlockEffect::EndShadow() const
{
    shadowShader_         .Unbind();
    shadowUniforms_       .Unbind();
    shadowInputLayout_    .Unbind();
    shadowRasterizerState_.Unbind();
}

std::unique_ptr<ModelBuilder> DefaultBlockEffect::CreateModelBuilder(const Vec3i &globalSectionPosition) const
{
    return std::make_unique<Builder>(globalSectionPosition, this);
}

void DefaultBlockEffect::SetForwardRenderParams(const ForwardRenderParams &params) const
{
    forwardShadowMapping_->SetRenderParams(params);
    forwardVSTransform_.SetValue({ params.camera->GetViewProjectionMatrix() });
    forwardPSPerFrame_.SetValue({ params.skyLight, 0 });
}

void DefaultBlockEffect::SetShadowRenderParams(const ShadowRenderParams &params) const
{
    shadowVSTransform_.SetValue({ params.shadowViewProj });
}

void DefaultBlockEffect::InitializeForward()
{
    D3D_SHADER_MACRO macros[2] = { GetShadowMappingEnableMacro(), { nullptr, nullptr } };

    forwardShader_.InitializeStageFromFile<SS_VS>(
        GLOBAL_CONFIG.ASSET_PATH["BlockEffect"]["Default"]["ForwardVertexShader"], macros);
    forwardShader_.InitializeStageFromFile<SS_PS>(
        GLOBAL_CONFIG.ASSET_PATH["BlockEffect"]["Default"]["ForwardPixelShader"], macros);
    if(!forwardShader_.IsAllStagesAvailable())
    {
        throw VRPGGameException("failed to initialize default block effect shader");
    }

    forwardUniforms_ = forwardShader_.CreateUniformManager();

    forwardInputLayout_ = InputLayoutBuilder
        ("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, offsetof(Vertex, position))
        ("BRIGHTNESS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, offsetof(Vertex, brightness))
        ("NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, offsetof(Vertex, normal))
        .Build(forwardShader_.GetVertexShaderByteCode());

    forwardVSTransform_.Initialize(true, nullptr);
    forwardUniforms_.GetConstantBufferSlot<SS_VS>("Transform")->SetBuffer(forwardVSTransform_);

    forwardPSPerFrame_.Initialize(true, nullptr);
    forwardUniforms_.GetConstantBufferSlot<SS_PS>("PerFrame")->SetBuffer(forwardPSPerFrame_);
}

void DefaultBlockEffect::InitializeShadow()
{
    std::string vertexShaderSource = agz::file::read_txt_file(
        GLOBAL_CONFIG.ASSET_PATH["BlockEffect"]["Default"]["ShadowVertexShader"]);
    std::string pixelShaderSource  = agz::file::read_txt_file(
        GLOBAL_CONFIG.ASSET_PATH["BlockEffect"]["Default"]["ShadowPixelShader"]);

    shadowShader_.InitializeStage<SS_VS>(vertexShaderSource);
    shadowShader_.InitializeStage<SS_PS>(pixelShaderSource);
    if(!shadowShader_.IsAllStagesAvailable())
    {
        throw VRPGGameException("failed to initialize shadow shader for default block effect");
    }

    shadowUniforms_ = shadowShader_.CreateUniformManager();

    shadowVSTransform_.Initialize(true, nullptr);
    shadowUniforms_.GetConstantBufferSlot<SS_VS>("Transform")->SetBuffer(shadowVSTransform_);

    shadowInputLayout_ = InputLayoutBuilder
        ("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, offsetof(Vertex, position))
        .Build(shadowShader_.GetVertexShaderByteCode());

    shadowRasterizerState_ = CreateRasterizerStateForShadowMapping();
}

VRPG_GAME_END
