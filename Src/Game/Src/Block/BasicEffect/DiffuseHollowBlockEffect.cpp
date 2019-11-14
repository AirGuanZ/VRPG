#include <agz/utility/file.h>

#include <VRPG/Game/Block/BasicEffect/DiffuseHollowBlockEffect.h>
#include <VRPG/Game/Misc/ShadowMappingRasterizerState.h>

VRPG_GAME_BEGIN

void DiffuseHollowBlockEffectGenerator::CommonProperties::InitializeForward()
{
    std::string vertexShaderSource = agz::file::read_txt_file("Asset/World/Shader/BlockEffect/DiffuseHollowVertex.hlsl");
    std::string pixelShaderSource = agz::file::read_txt_file("Asset/World/Shader/BlockEffect/DiffuseHollowPixel.hlsl");
    forwardShader_.InitializeStage<SS_VS>(vertexShaderSource);
    forwardShader_.InitializeStage<SS_PS>(pixelShaderSource);
    if(!forwardShader_.IsAllStagesAvailable())
        throw VRPGGameException("failed to initialize diffuse hollow block effect shader");

    forwardUniforms_ = forwardShader_.CreateUniformManager();

    forwardInputLayout_ = InputLayoutBuilder
        ("POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT,    offsetof(Vertex, position))
        ("TEXCOORD",   0, DXGI_FORMAT_R32G32_FLOAT,       offsetof(Vertex, texCoord))
        ("NORMAL",     0, DXGI_FORMAT_R32G32B32_FLOAT,    offsetof(Vertex, normal))
        ("TEXINDEX",   0, DXGI_FORMAT_R32_UINT,           offsetof(Vertex, texIndex))
        ("BRIGHTNESS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, offsetof(Vertex, brightness))
        .Build(forwardShader_.GetVertexShaderByteCode());

    forwardVSTransform_.Initialize(true, nullptr);
    forwardPSPerFrame_.Initialize(true, nullptr);

    forwardUniforms_.GetConstantBufferSlot<SS_VS>("Transform")->SetBuffer(forwardVSTransform_);
    forwardUniforms_.GetConstantBufferSlot<SS_PS>("PerFrame")->SetBuffer(forwardPSPerFrame_);

    Sampler diffuseSampler;
    diffuseSampler.Initialize(D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR);
    forwardUniforms_.GetSamplerSlot<SS_PS>("DiffuseSampler")->SetSampler(diffuseSampler);

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

    forwardDiffuseTextureSlot_ = forwardUniforms_.GetShaderResourceSlot<SS_PS>("DiffuseTexture");
    if(!forwardDiffuseTextureSlot_)
        throw VRPGGameException("shader resource slot not found in diffuse hollow block effect shader: DiffuseTexture");

    forwardShadowMapSlot_ = forwardUniforms_.GetShaderResourceSlot<SS_PS>("ShadowMap");
    if(!forwardShadowMapSlot_)
        throw VRPGGameException("shader resource slot not found in diffuse hollow block effect shader: ShadowMap");

    forwardRasterizerState_.Initialize(D3D11_FILL_SOLID, D3D11_CULL_NONE, false);
}

void DiffuseHollowBlockEffectGenerator::CommonProperties::InitializeShadow()
{
    const char *vertexShaderSource = R"___(
cbuffer Transform
{
    float4x4 VP;
};

struct VSInput
{
    float3 position : POSITION;
    float2 texCoord : TEXCOORD;
    nointerpolation uint texIndex : TEXINDEX;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
    nointerpolation uint texIndex : TEXINDEX;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput)0;
    output.position = mul(float4(input.position, 1), VP);
    output.texCoord = input.texCoord;
    output.texIndex = input.texIndex;
    return output;
}
)___";

    const char *pixelShaderSource = R"___(
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
)___";

    shadowShader_.InitializeStage<SS_VS>(vertexShaderSource);
    shadowShader_.InitializeStage<SS_PS>(pixelShaderSource);
    if(!shadowShader_.IsAllStagesAvailable())
        throw VRPGGameException("failed to initialize shadow shader for transparent block effect");

    shadowUniforms_ = shadowShader_.CreateUniformManager();

    shadowVSTransform_.Initialize(true, nullptr);
    shadowUniforms_.GetConstantBufferSlot<SS_VS>("Transform")->SetBuffer(shadowVSTransform_);

    Sampler sampler;
    sampler.Initialize(D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR);
    shadowUniforms_.GetSamplerSlot<SS_PS>("DiffuseSampler")->SetSampler(sampler);

    shadowDiffuseTextureSlot_ = shadowUniforms_.GetShaderResourceSlot<SS_PS>("DiffuseTexture");

    shadowInputLayout_ = InputLayoutBuilder
        ("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, offsetof(Vertex, position))
        ("TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    offsetof(Vertex, texCoord))
        ("TEXINDEX", 0, DXGI_FORMAT_R32_UINT,        offsetof(Vertex, texIndex))
        .Build(shadowShader_.GetVertexShaderByteCode());

    shadowRasterizerState_ = CreateRasterizerStateForShadowMapping(false);
}

DiffuseHollowBlockEffectGenerator::CommonProperties::CommonProperties()
    : forwardDiffuseTextureSlot_(nullptr), forwardShadowMapSlot_(nullptr),
      shadowDiffuseTextureSlot_(nullptr)
{
    InitializeForward();
    InitializeShadow();
}

DiffuseHollowBlockEffectGenerator::DiffuseHollowBlockEffectGenerator(int textureSize, int expectedArraySize)
{
    assert(textureSize > 0 && expectedArraySize > 0);
    commonProperties_ = std::make_shared<CommonProperties>();
    textureSize_ = textureSize;
    maxArraySize_ = (std::min)(expectedArraySize, D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION);
    generatedEffectCount_ = 0;
}

bool DiffuseHollowBlockEffectGenerator::IsEmpty() const noexcept
{
    return textureArrayData_.empty();
}

bool DiffuseHollowBlockEffectGenerator::HasEnoughSpaceFor(int arrayDataCount) const noexcept
{
    return int(textureArrayData_.size()) + arrayDataCount <= maxArraySize_;
}

int DiffuseHollowBlockEffectGenerator::AddTexture(const Vec4 *data)
{
    int ret = int(textureArrayData_.size());
    textureArrayData_.emplace_back(textureSize_, textureSize_, data);
    return ret;
}

void DiffuseHollowBlockEffectGenerator::InitializeEffect(DiffuseHollowBlockEffect &effect)
{
    assert(!textureArrayData_.empty());

    // 生成mipmap chain

    std::vector<agz::texture::mipmap_chain_t<Vec4>> mipmapChains(textureArrayData_.size());
    for(size_t i = 0; i < textureArrayData_.size(); ++i)
    {
        auto &textureData = textureArrayData_[i];
        mipmapChains[i].generate(textureData);
    }

    // 创建texture和srv

    D3D11_TEXTURE2D_DESC textureDesc;
    textureDesc.Width              = UINT(textureSize_);
    textureDesc.Height             = UINT(textureSize_);
    textureDesc.MipLevels          = UINT(mipmapChains[0].chain_length());;
    textureDesc.ArraySize          = UINT(textureArrayData_.size());
    textureDesc.Format             = DXGI_FORMAT_R32G32B32A32_FLOAT;
    textureDesc.SampleDesc.Count   = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage              = D3D11_USAGE_IMMUTABLE;
    textureDesc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags     = 0;
    textureDesc.MiscFlags          = 0;

    std::vector<D3D11_SUBRESOURCE_DATA> initDataArr(textureArrayData_.size() * mipmapChains[0].chain_length());
    int initDataArrIndex = 0;
    for(size_t i = 0; i < textureArrayData_.size(); ++i)
    {
        for(int j = 0; j < mipmapChains[i].chain_length(); ++j)
        {
            auto &initData = initDataArr[initDataArrIndex++];
            auto &mipmapData = mipmapChains[i].chain_elem(j);

            initData.pSysMem          = mipmapData.raw_data();
            initData.SysMemPitch      = mipmapData.width() * sizeof(Vec4);
            initData.SysMemSlicePitch = 0;
        }
    }

    ComPtr<ID3D11Texture2D> texture = Base::D3D::CreateTexture2D(textureDesc, initDataArr.data());
    if(!texture)
        throw VRPGGameException("failed to create texture2d array for diffuse hollow block effect");

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format                         = DXGI_FORMAT_R32G32B32A32_FLOAT;
    srvDesc.ViewDimension                  = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    srvDesc.Texture2DArray.MostDetailedMip = 0;
    srvDesc.Texture2DArray.MipLevels       = -1;
    srvDesc.Texture2DArray.FirstArraySlice = 0;
    srvDesc.Texture2DArray.ArraySize       = UINT(textureArrayData_.size());

    ComPtr<ID3D11ShaderResourceView> srv = Base::D3D::CreateShaderResourceView(srvDesc, texture.Get());
    if(!srv)
        throw VRPGGameException("failed to create shader resource view of texture2d array of for diffuse hollow block effect");

    effect.Initialize(commonProperties_, ShaderResourceView(srv), generatedEffectCount_++);
}

void DiffuseHollowBlockEffect::Initialize(
    std::shared_ptr<Generator::CommonProperties> commonProperties,
    ShaderResourceView textureArray, int semanticsIndex)
{
    commonProperties_ = std::move(commonProperties);
    textureArray_ = std::move(textureArray);
    name_ = "diffuse hollow " + std::to_string(semanticsIndex);
}

const char *DiffuseHollowBlockEffect::GetName() const
{
    return name_.c_str();
}

bool DiffuseHollowBlockEffect::IsTransparent() const noexcept
{
    return false;
}

void DiffuseHollowBlockEffect::StartForward() const
{
    commonProperties_->forwardDiffuseTextureSlot_->SetShaderResourceView(textureArray_);
    commonProperties_->forwardShader_.Bind();
    commonProperties_->forwardUniforms_.Bind();
    commonProperties_->forwardInputLayout_.Bind();
    commonProperties_->forwardRasterizerState_.Bind();
}

void DiffuseHollowBlockEffect::EndForward() const
{
    commonProperties_->forwardRasterizerState_.Unbind();
    commonProperties_->forwardShader_.Unbind();
    commonProperties_->forwardUniforms_.Unbind();
    commonProperties_->forwardInputLayout_.Unbind();
}

void DiffuseHollowBlockEffect::StartShadow() const
{
    commonProperties_->shadowDiffuseTextureSlot_->SetShaderResourceView(textureArray_);
    commonProperties_->shadowShader_.Bind();
    commonProperties_->shadowUniforms_.Bind();
    commonProperties_->shadowInputLayout_.Bind();
    commonProperties_->shadowRasterizerState_.Bind();
}

void DiffuseHollowBlockEffect::EndShadow() const
{
    commonProperties_->shadowShader_.Unbind();
    commonProperties_->shadowUniforms_.Unbind();
    commonProperties_->shadowInputLayout_.Unbind();
    commonProperties_->shadowRasterizerState_.Unbind();
}

std::unique_ptr<PartialSectionModelBuilder> DiffuseHollowBlockEffect::CreateModelBuilder(const Vec3i &globalSectionPosition) const
{
    return std::make_unique<Builder>(globalSectionPosition, this);
}

void DiffuseHollowBlockEffect::SetForwardRenderParams(const BlockForwardRenderParams &params) const
{
    commonProperties_->forwardShadowMapSlot_->SetShaderResourceView(params.shadowMapSRV.Get());
    commonProperties_->forwardVSTransform_.SetValue({ params.shadowViewProj, params.camera->GetViewProjectionMatrix() });
    commonProperties_->forwardPSPerFrame_.SetValue({ params.skyLight, params.shadowScale, params.sunlightDirection, params.dx });
}

void DiffuseHollowBlockEffect::SetShadowRenderParams(const BlockShadowRenderParams &params) const
{
    commonProperties_->shadowVSTransform_.SetValue({ params.shadowViewProj });
}

VRPG_GAME_END
