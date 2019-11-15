#include <agz/utility/file.h>

#include <VRPG/Game/Block/BasicEffect/DiffuseHollowBlockEffect.h>
#include <VRPG/Game/Misc/ShadowMappingRasterizerState.h>

VRPG_GAME_BEGIN

void DiffuseHollowBlockEffectCommon::InitializeForward()
{
    std::string vertexShaderSource = agz::file::read_txt_file("Asset/World/Shader/BlockEffect/DiffuseHollow/DiffuseHollowVertex.hlsl");
    std::string pixelShaderSource = agz::file::read_txt_file("Asset/World/Shader/BlockEffect/DiffuseHollow/DiffuseHollowPixel.hlsl");
    forwardShader_.InitializeStage<SS_VS>(vertexShaderSource);
    forwardShader_.InitializeStage<SS_PS>(pixelShaderSource);
    if(!forwardShader_.IsAllStagesAvailable())
        throw VRPGGameException("failed to initialize diffuse hollow block effect shader");

    forwardUniforms_ = forwardShader_.CreateUniformManager();

    forwardInputLayout_ = InputLayoutBuilder
        ("POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT,    offsetof(DiffuseHollowBlockEffect::Vertex, position))
        ("TEXCOORD",   0, DXGI_FORMAT_R32G32_FLOAT,       offsetof(DiffuseHollowBlockEffect::Vertex, texCoord))
        ("NORMAL",     0, DXGI_FORMAT_R32G32B32_FLOAT,    offsetof(DiffuseHollowBlockEffect::Vertex, normal))
        ("TEXINDEX",   0, DXGI_FORMAT_R32_UINT,           offsetof(DiffuseHollowBlockEffect::Vertex, texIndex))
        ("BRIGHTNESS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, offsetof(DiffuseHollowBlockEffect::Vertex, brightness))
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

void DiffuseHollowBlockEffectCommon::InitializeShadow()
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
        ("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, offsetof(DiffuseHollowBlockEffect::Vertex, position))
        ("TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    offsetof(DiffuseHollowBlockEffect::Vertex, texCoord))
        ("TEXINDEX", 0, DXGI_FORMAT_R32_UINT,        offsetof(DiffuseHollowBlockEffect::Vertex, texIndex))
        .Build(shadowShader_.GetVertexShaderByteCode());

    shadowRasterizerState_ = CreateRasterizerStateForShadowMapping(false);
}

DiffuseHollowBlockEffectCommon::DiffuseHollowBlockEffectCommon()
    : forwardDiffuseTextureSlot_(nullptr), forwardShadowMapSlot_(nullptr), shadowDiffuseTextureSlot_(nullptr)
{
    InitializeForward();
    InitializeShadow();
}

void DiffuseHollowBlockEffectCommon::SetForwardRenderParams(const BlockForwardRenderParams &params)
{
    forwardShadowMapSlot_->SetShaderResourceView(params.shadowMapSRV.Get());
    forwardVSTransform_.SetValue({ params.shadowViewProj, params.camera->GetViewProjectionMatrix() });
    forwardPSPerFrame_.SetValue({ params.skyLight, params.shadowScale, params.sunlightDirection, params.PCFStep });
}

void DiffuseHollowBlockEffectCommon::SetShadowRenderParams(const BlockShadowRenderParams &params)
{
    shadowVSTransform_.SetValue({ params.shadowViewProj });
}

void DiffuseHollowBlockEffectCommon::StartForward(ID3D11ShaderResourceView *textureArray)
{
    forwardDiffuseTextureSlot_->SetShaderResourceView(textureArray);
    forwardShader_.Bind();
    forwardUniforms_.Bind();
    forwardInputLayout_.Bind();
    forwardRasterizerState_.Bind();
}

void DiffuseHollowBlockEffectCommon::EndForward()
{
    forwardRasterizerState_.Unbind();
    forwardShader_.Unbind();
    forwardUniforms_.Unbind();
    forwardInputLayout_.Unbind();
}

void DiffuseHollowBlockEffectCommon::StartShadow(ID3D11ShaderResourceView *textureArray)
{
    shadowDiffuseTextureSlot_->SetShaderResourceView(textureArray);
    shadowShader_.Bind();
    shadowUniforms_.Bind();
    shadowInputLayout_.Bind();
    shadowRasterizerState_.Bind();
}

void DiffuseHollowBlockEffectCommon::EndShadow()
{
    shadowShader_.Unbind();
    shadowUniforms_.Unbind();
    shadowInputLayout_.Unbind();
    shadowRasterizerState_.Unbind();
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
    common_->StartForward(textureArray_.Get());
}

void DiffuseHollowBlockEffect::EndForward() const
{
    common_->EndForward();
}

void DiffuseHollowBlockEffect::StartShadow() const
{
    common_->StartShadow(textureArray_.Get());
}

void DiffuseHollowBlockEffect::EndShadow() const
{
    common_->EndShadow();
}

std::unique_ptr<PartialSectionModelBuilder> DiffuseHollowBlockEffect::CreateModelBuilder(const Vec3i &globalSectionPosition) const
{
    return std::make_unique<Builder>(globalSectionPosition, this);
}

void DiffuseHollowBlockEffect::SetForwardRenderParams(const BlockForwardRenderParams &params) const
{
    common_->SetForwardRenderParams(params);
}

void DiffuseHollowBlockEffect::SetShadowRenderParams(const BlockShadowRenderParams &params) const
{
    common_->SetShadowRenderParams(params);
}

void DiffuseHollowBlockEffect::Initialize(
    std::shared_ptr<DiffuseHollowBlockEffectCommon> commonProperties,
    ShaderResourceView textureArray, std::string name)
{
    common_ = std::move(commonProperties);
    textureArray_ = std::move(textureArray);
    name_ = std::move(name);
}

DiffuseHollowBlockEffectGenerator::DiffuseHollowBlockEffectGenerator(int textureSize, int maxArraySize)
{
    assert(textureSize > 0 && maxArraySize > 0);

    common_                   = std::make_shared<DiffuseHollowBlockEffectCommon>();
    textureSize_              = textureSize;
    maxArraySize_             = (std::min)(maxArraySize, D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION);
    nextEffectSemanticsIndex_ = 0;

    currentEffect_ = std::make_shared<DiffuseHollowBlockEffect>();
}

std::shared_ptr<DiffuseHollowBlockEffect> DiffuseHollowBlockEffectGenerator::GetEffectWithTextureSpaces(int textureCount)
{
    if(static_cast<int>(textureArrayData_.size()) + textureCount <= maxArraySize_)
        return currentEffect_;

    InitializeCurrentEffect();
    return currentEffect_;
}

int DiffuseHollowBlockEffectGenerator::AddTexture(const Vec4 *textureData)
{
    int ret = static_cast<int>(textureArrayData_.size());
    textureArrayData_.emplace_back(textureSize_, textureSize_, textureData);
    return ret;
}

void DiffuseHollowBlockEffectGenerator::Done()
{
    InitializeCurrentEffect();
}

void DiffuseHollowBlockEffectGenerator::InitializeCurrentEffect()
{
    if(textureArrayData_.empty())
        return;

    
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

    // 延迟初始化之前的effect

    std::string name = "diffuse hollow " + std::to_string(nextEffectSemanticsIndex_);
    currentEffect_->Initialize(common_, ShaderResourceView(srv), std::move(name));
    BlockEffectManager::GetInstance().RegisterBlockEffect(currentEffect_);

    // 准备新的block effect

    textureArrayData_.clear();
    currentEffect_ = std::make_shared<DiffuseHollowBlockEffect>();
}

VRPG_GAME_END
