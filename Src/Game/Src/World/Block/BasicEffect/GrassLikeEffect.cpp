#include <agz/utility/file.h>

#include <VRPG/Game/Config/GlobalConfig.h>
#include <VRPG/Game/World/Block/BasicEffect/GrassLikeEffect.h>

VRPG_GAME_BEGIN

void GrassLikeEffectCommon::InitializeForward()
{
    D3D_SHADER_MACRO macros[2] = { GetShadowMappingEnableMacro(), { nullptr, nullptr } };

    forwardShader_.InitializeStageFromFile<SS_VS>(
        GLOBAL_CONFIG.ASSET_PATH["BlockEffect"]["GrassLike"]["ForwardVertexShader"], macros);
    forwardShader_.InitializeStageFromFile<SS_PS>(
        GLOBAL_CONFIG.ASSET_PATH["BlockEffect"]["GrassLike"]["ForwardPixelShader"], macros);
    if(!forwardShader_.IsAllStagesAvailable())
    {
        throw VRPGGameException("failed to initialize diffuse hollow block effect shader");
    }

    forwardUniforms_ = forwardShader_.CreateUniformManager();

    forwardInputLayout_ = InputLayoutBuilder
        ("POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT,    offsetof(GrassLikeEffect::Vertex, position))
        ("TEXCOORD",   0, DXGI_FORMAT_R32G32_FLOAT,       offsetof(GrassLikeEffect::Vertex, texCoord))
        ("NORMAL",     0, DXGI_FORMAT_R32G32B32_FLOAT,    offsetof(GrassLikeEffect::Vertex, normal))
        ("TEXINDEX",   0, DXGI_FORMAT_R32_UINT,           offsetof(GrassLikeEffect::Vertex, texIndex))
        ("BRIGHTNESS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, offsetof(GrassLikeEffect::Vertex, brightness))
        .Build(forwardShader_.GetVertexShaderByteCode());

    forwardVSTransform_.Initialize(true, nullptr);
    forwardPSPerFrame_.Initialize(true, nullptr);

    forwardUniforms_.GetConstantBufferSlot<SS_VS>("Transform")->SetBuffer(forwardVSTransform_);
    forwardUniforms_.GetConstantBufferSlot<SS_PS>("PerFrame")->SetBuffer(forwardPSPerFrame_);

    Sampler diffuseSampler;
    diffuseSampler.Initialize(D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR);
    forwardUniforms_.GetSamplerSlot<SS_PS>("DiffuseSampler")->SetSampler(diffuseSampler);

    forwardDiffuseTextureSlot_ = forwardUniforms_.GetShaderResourceSlot<SS_PS>("DiffuseTexture");
    if(!forwardDiffuseTextureSlot_)
    {
        throw VRPGGameException("shader resource slot not found in grass like block effect shader: DiffuseTexture");
    }
}

void GrassLikeEffectCommon::InitializeShadow()
{
    std::string vertexShaderSource = agz::file::read_txt_file(
        GLOBAL_CONFIG.ASSET_PATH["BlockEffect"]["GrassLike"]["ShadowVertexShader"]);
    std::string pixelShaderSource  = agz::file::read_txt_file(
        GLOBAL_CONFIG.ASSET_PATH["BlockEffect"]["GrassLike"]["ShadowPixelShader"]);

    shadowShader_.InitializeStage<SS_VS>(vertexShaderSource);
    shadowShader_.InitializeStage<SS_PS>(pixelShaderSource);
    if(!shadowShader_.IsAllStagesAvailable())
    {
        throw VRPGGameException("failed to initialize shadow shader for grass like block effect");
    }

    shadowUniforms_ = shadowShader_.CreateUniformManager();

    shadowVSTransform_.Initialize(true, nullptr);
    shadowUniforms_.GetConstantBufferSlot<SS_VS>("Transform")->SetBuffer(shadowVSTransform_);

    Sampler sampler;
    sampler.Initialize(D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR);
    shadowUniforms_.GetSamplerSlot<SS_PS>("DiffuseSampler")->SetSampler(sampler);

    shadowDiffuseTextureSlot_ = shadowUniforms_.GetShaderResourceSlot<SS_PS>("DiffuseTexture");

    shadowInputLayout_ = InputLayoutBuilder
        ("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, offsetof(GrassLikeEffect::Vertex, position))
        ("TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    offsetof(GrassLikeEffect::Vertex, texCoord))
        ("TEXINDEX", 0, DXGI_FORMAT_R32_UINT,        offsetof(GrassLikeEffect::Vertex, texIndex))
        .Build(shadowShader_.GetVertexShaderByteCode());

    shadowRasterizerState_ = CreateRasterizerStateForShadowMapping(false);
}

GrassLikeEffectCommon::GrassLikeEffectCommon()
    : forwardDiffuseTextureSlot_(nullptr), shadowDiffuseTextureSlot_(nullptr)
{
    InitializeForward();
    InitializeShadow();

    forwardShadowMapping_ = CreateForwardshadowMapping(&forwardUniforms_);
}

void GrassLikeEffectCommon::SetForwardRenderParams(const ForwardRenderParams &params)
{
    forwardShadowMapping_->SetRenderParams(params);
    forwardVSTransform_.SetValue({
        params.camera->GetViewProjectionMatrix() });
    forwardPSPerFrame_.SetValue({ params.skyLight, 0 });
}

void GrassLikeEffectCommon::SetShadowRenderParams(const ShadowRenderParams &params)
{
    shadowVSTransform_.SetValue({ params.shadowViewProj });
}

void GrassLikeEffectCommon::StartForward(ID3D11ShaderResourceView *textureArray)
{
    forwardShadowMapping_->Bind();
    forwardDiffuseTextureSlot_->SetShaderResourceView(textureArray);
    forwardShader_     .Bind();
    forwardUniforms_   .Bind();
    forwardInputLayout_.Bind();
}

void GrassLikeEffectCommon::EndForward()
{
    forwardShader_     .Unbind();
    forwardUniforms_   .Unbind();
    forwardInputLayout_.Unbind();
    forwardShadowMapping_->Unbind();
}

void GrassLikeEffectCommon::StartShadow(ID3D11ShaderResourceView *textureArray)
{
    shadowDiffuseTextureSlot_->SetShaderResourceView(textureArray);
    shadowShader_         .Bind();
    shadowUniforms_       .Bind();
    shadowInputLayout_    .Bind();
    shadowRasterizerState_.Bind();
}

void GrassLikeEffectCommon::EndShadow()
{
    shadowShader_         .Unbind();
    shadowUniforms_       .Unbind();
    shadowInputLayout_    .Unbind();
    shadowRasterizerState_.Unbind();
}

const char *GrassLikeEffect::GetName() const
{
    return name_.c_str();
}

bool GrassLikeEffect::IsTransparent() const noexcept
{
    return false;
}

void GrassLikeEffect::StartForward() const
{
    common_->StartForward(textureArray_.Get());
}

void GrassLikeEffect::EndForward() const
{
    common_->EndForward();
}

void GrassLikeEffect::StartShadow() const
{
    common_->StartShadow(textureArray_.Get());
}

void GrassLikeEffect::EndShadow() const
{
    common_->EndShadow();
}

std::unique_ptr<ModelBuilder> GrassLikeEffect::CreateModelBuilder(const Vec3i &globalSectionPosition) const
{
    return std::make_unique<Builder>(globalSectionPosition, this);
}

void GrassLikeEffect::SetForwardRenderParams(const ForwardRenderParams &params) const
{
    common_->SetForwardRenderParams(params);
}

void GrassLikeEffect::SetShadowRenderParams(const ShadowRenderParams &params) const
{
    common_->SetShadowRenderParams(params);
}

void GrassLikeEffect::Initialize(
    std::shared_ptr<GrassLikeEffectCommon> commonProperties,
    ShaderResourceView textureArray, std::string name)
{
    common_ = std::move(commonProperties);
    textureArray_ = std::move(textureArray);
    name_ = std::move(name);
}

GrassLikeEffectGenerator::GrassLikeEffectGenerator(int textureSize, int maxArraySize)
{
    assert(textureSize > 0 && maxArraySize > 0);

    common_                   = std::make_shared<GrassLikeEffectCommon>();
    textureSize_              = textureSize;
    maxArraySize_             = (std::min)(maxArraySize, D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION);
    nextEffectSemanticsIndex_ = 0;

    currentEffect_ = std::make_shared<GrassLikeEffect>();
}

std::shared_ptr<GrassLikeEffect> GrassLikeEffectGenerator::GetEffectWithTextureSpaces(int textureCount)
{
    if(static_cast<int>(textureArrayData_.size()) + textureCount <= maxArraySize_)
    {
        return currentEffect_;
    }

    InitializeCurrentEffect();
    return currentEffect_;
}

int GrassLikeEffectGenerator::AddTexture(const Vec4 *textureData)
{
    int ret = static_cast<int>(textureArrayData_.size());
    textureArrayData_.emplace_back(textureSize_, textureSize_, textureData);
    return ret;
}

void GrassLikeEffectGenerator::Done()
{
    InitializeCurrentEffect();
}

void GrassLikeEffectGenerator::InitializeCurrentEffect()
{
    if(textureArrayData_.empty())
    {
        return;
    }

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
    {
        throw VRPGGameException("failed to create texture2d array for grass like block effect");
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format                         = DXGI_FORMAT_R32G32B32A32_FLOAT;
    srvDesc.ViewDimension                  = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    srvDesc.Texture2DArray.MostDetailedMip = 0;
    srvDesc.Texture2DArray.MipLevels       = -1;
    srvDesc.Texture2DArray.FirstArraySlice = 0;
    srvDesc.Texture2DArray.ArraySize       = UINT(textureArrayData_.size());

    ComPtr<ID3D11ShaderResourceView> srv = Base::D3D::CreateShaderResourceView(srvDesc, texture.Get());
    if(!srv)
    {
        throw VRPGGameException("failed to create shader resource view of texture2d array of for grass like block effect");
    }

    // 延迟初始化之前的effect

    std::string name = "grass like " + std::to_string(nextEffectSemanticsIndex_);
    currentEffect_->Initialize(common_, ShaderResourceView(srv), std::move(name));
    BlockEffectManager::GetInstance().RegisterBlockEffect(currentEffect_);

    // 准备新的block effect

    textureArrayData_.clear();
    currentEffect_ = std::make_shared<GrassLikeEffect>();
}

VRPG_GAME_END
