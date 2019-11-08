#include <agz/utility/file.h>

#include <VRPG/Game/Block/BasicEffect/DefaultBlockEffect.h>

VRPG_GAME_BEGIN

DefaultBlockEffect::DefaultBlockEffect()
{
    std::string vertexShaderSource = agz::file::read_txt_file("Asset/World/Shader/BlockEffect/DefaultVertex.hlsl");
    std::string pixelShaderSource  = agz::file::read_txt_file("Asset/World/Shader/BlockEffect/DefaultPixel.hlsl");
    shader_.InitializeStage<SS_VS>(vertexShaderSource);
    shader_.InitializeStage<SS_PS>(pixelShaderSource);
    if(!shader_.IsAllStagesAvailable())
        throw VRPGWorldException("failed to initialize default block effect shader");

    uniforms_ = shader_.CreateUniformManager();

    inputLayout_ = InputLayoutBuilder
        ("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, ByteOffset(&Vertex::position))
        ("BRIGHTNESS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, ByteOffset(&Vertex::brightness))
        .Build(shader_.GetVertexShaderByteCode());

    vsTransform_.Initialize(true, nullptr);
    uniforms_.GetConstantBufferSlot<SS_VS>("Transform")->SetBuffer(vsTransform_);

    psSky_.Initialize(true, nullptr);
    uniforms_.GetConstantBufferSlot<SS_PS>("Sky")->SetBuffer(psSky_);
}

const char *DefaultBlockEffect::GetName() const
{
    return "default";
}

bool DefaultBlockEffect::IsTransparent() const noexcept
{
    return false;
}

void DefaultBlockEffect::Bind() const
{
    shader_.Bind();
    uniforms_.Bind();
    inputLayout_.Bind();
}

void DefaultBlockEffect::Unbind() const
{
    inputLayout_.Unbind();
    uniforms_.Unbind();
    shader_.Unbind();
}

std::unique_ptr<PartialSectionModelBuilder> DefaultBlockEffect::CreateModelBuilder(const Vec3i &globalSectionPosition) const
{
    return std::make_unique<Builder>(globalSectionPosition, this);
}

void DefaultBlockEffect::SetRenderParams(const BlockRenderParams &params) const
{
    vsTransform_.SetValue({ params.camera->GetViewProjectionMatrix() });
    psSky_.SetValue({ params.skyLight, 0 });
}

VRPG_GAME_END
