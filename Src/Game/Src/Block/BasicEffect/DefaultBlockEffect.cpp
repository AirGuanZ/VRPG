#include <agz/utility/file.h>

#include <VRPG/Game/Block/BasicEffect/DefaultBlockEffect.h>

VRPG_GAME_BEGIN

DefaultBlockEffect::DefaultBlockEffect()
{
    std::string vertexShaderSource = agz::file::read_txt_file("Asset/World/Shader/BlockEffect/DefaultVertex.hlsl");
    std::string pixelShaderSource  = agz::file::read_txt_file("Asset/World/Shader/BlockEffect/DefaultPixel.hlsl");
    forwardShader_.InitializeStage<SS_VS>(vertexShaderSource);
    forwardShader_.InitializeStage<SS_PS>(pixelShaderSource);
    if(!forwardShader_.IsAllStagesAvailable())
        throw VRPGGameException("failed to initialize default block effect shader");

    forwardUniforms_ = forwardShader_.CreateUniformManager();

    forwardInputLayout_ = InputLayoutBuilder
        ("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, offsetof(Vertex, position))
        ("BRIGHTNESS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, offsetof(Vertex, brightness))
        .Build(forwardShader_.GetVertexShaderByteCode());

    forwardVSTransform_.Initialize(true, nullptr);
    forwardUniforms_.GetConstantBufferSlot<SS_VS>("Transform")->SetBuffer(forwardVSTransform_);

    forwardPSSky_.Initialize(true, nullptr);
    forwardUniforms_.GetConstantBufferSlot<SS_PS>("Sky")->SetBuffer(forwardPSSky_);
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

std::unique_ptr<PartialSectionModelBuilder> DefaultBlockEffect::CreateModelBuilder(const Vec3i &globalSectionPosition) const
{
    return std::make_unique<Builder>(globalSectionPosition, this);
}

void DefaultBlockEffect::SetForwardRenderParams(const BlockForwardRenderParams &params) const
{
    forwardVSTransform_.SetValue({ params.camera->GetViewProjectionMatrix() });
    forwardPSSky_.SetValue({ params.skyLight, 0 });
}

VRPG_GAME_END
