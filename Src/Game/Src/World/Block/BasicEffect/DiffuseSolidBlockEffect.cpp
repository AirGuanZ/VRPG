#include <agz/utility/file.h>

#include <VRPG/Game/Config/GlobalConfig.h>
#include <VRPG/Game/World/Block/BasicEffect/DiffuseSolidBlockEffect.h>

VRPG_GAME_BEGIN

void DiffuseSolidBlockEffectCommon::InitializeForward()
{
    D3D_SHADER_MACRO macros[2] = { GetShadowMappingEnableMacro(), { nullptr, nullptr } };

    forwardShader.InitializeStageFromFile<SS_VS>(
        GLOBAL_CONFIG.ASSET_PATH["BlockEffect"]["DiffuseSolid"]["ForwardVertexShader"], macros);
    forwardShader.InitializeStageFromFile<SS_PS>(
        GLOBAL_CONFIG.ASSET_PATH["BlockEffect"]["DiffuseSolid"]["ForwardPixelShader"], macros);
    if(!forwardShader.IsAllStagesAvailable())
    {
        throw VRPGGameException("failed to initialize diffuse solid block effect shader (forward)");
    }

    forwardUniforms = forwardShader.CreateUniformManager();

    forwardInputLayout = InputLayoutBuilder
        ("POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT,    offsetof(DiffuseSolidBlockEffect::Vertex, position))
        ("NORMAL",     0, DXGI_FORMAT_R32G32B32_FLOAT,    offsetof(DiffuseSolidBlockEffect::Vertex, normal))
        ("TEXCOORD",   0, DXGI_FORMAT_R32G32_FLOAT,       offsetof(DiffuseSolidBlockEffect::Vertex, texCoord))
        ("TEXINDEX",   0, DXGI_FORMAT_R32_UINT,           offsetof(DiffuseSolidBlockEffect::Vertex, texIndex))
        ("BRIGHTNESS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, offsetof(DiffuseSolidBlockEffect::Vertex, brightness))
        .Build(forwardShader.GetVertexShaderByteCode());

    forwardVSTransform.Initialize(true, nullptr);
    forwardPSPerFrame.Initialize(true, nullptr);

    forwardUniforms.GetConstantBufferSlot<SS_VS>("Transform")->SetBuffer(forwardVSTransform);
    forwardUniforms.GetConstantBufferSlot<SS_PS>("PerFrame")->SetBuffer(forwardPSPerFrame);

    Sampler diffuseSampler;
    diffuseSampler.Initialize(D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR);
    forwardUniforms.GetSamplerSlot<SS_PS>("DiffuseSampler")->SetSampler(diffuseSampler);

    forwardDiffuseTextureSlot = forwardUniforms.GetShaderResourceSlot<SS_PS>("DiffuseTexture");
    if(!forwardDiffuseTextureSlot)
    {
        throw VRPGGameException("shader resource slot not found in diffuse solid block effect shader: DiffuseTexture");
    }
}

void DiffuseSolidBlockEffectCommon::InitializeShadow()
{
    std::string vertexShaderSource = agz::file::read_txt_file(
        GLOBAL_CONFIG.ASSET_PATH["BlockEffect"]["DiffuseSolid"]["ShadowVertexShader"]);
    std::string pixelShaderSource  = agz::file::read_txt_file(
        GLOBAL_CONFIG.ASSET_PATH["BlockEffect"]["DiffuseSolid"]["ShadowPixelShader"]);

    shadowShader.InitializeStage<SS_VS>(vertexShaderSource);
    shadowShader.InitializeStage<SS_PS>(pixelShaderSource);
    if(!shadowShader.IsAllStagesAvailable())
    {
        throw VRPGGameException("failed to initialize shadow shader for diffuse solid block effect");
    }

    shadowUniforms = shadowShader.CreateUniformManager();

    shadowVSTransform.Initialize(true, nullptr);
    shadowUniforms.GetConstantBufferSlot<SS_VS>("Transform")->SetBuffer(shadowVSTransform);

    shadowInputLayout = InputLayoutBuilder
        ("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, offsetof(DiffuseSolidBlockEffect::Vertex, position))
        .Build(shadowShader.GetVertexShaderByteCode());

    shadowRasterizerState = CreateRasterizerStateForShadowMapping();
}

DiffuseSolidBlockEffectCommon::DiffuseSolidBlockEffectCommon()
    : forwardDiffuseTextureSlot(nullptr)
{
    InitializeForward();
    InitializeShadow();

    forwardShadowMapping = CreateForwardshadowMapping(&forwardUniforms);
}

void DiffuseSolidBlockEffectCommon::SetForwardRenderParams(const ForwardRenderParams &params)
{
    forwardShadowMapping->SetRenderParams(params);
    forwardVSTransform.SetValue({ params.camera->GetViewProjectionMatrix() });
    forwardPSPerFrame.SetValue({ params.skyLight, 0 });
}

void DiffuseSolidBlockEffectCommon::SetShadowRenderParams(const ShadowRenderParams &params)
{
    shadowVSTransform.SetValue({ params.shadowViewProj });
}

void DiffuseSolidBlockEffectCommon::StartForward(ID3D11ShaderResourceView *diffuseTextureArray) const
{
    forwardShadowMapping->StartForward();

    forwardDiffuseTextureSlot->SetShaderResourceView(diffuseTextureArray);
    forwardShader     .Bind();
    forwardUniforms   .Bind();
    forwardInputLayout.Bind();
}

void DiffuseSolidBlockEffectCommon::EndForward() const
{
    forwardShader     .Unbind();
    forwardUniforms   .Unbind();
    forwardInputLayout.Unbind();

    forwardShadowMapping->EndForward();
}

void DiffuseSolidBlockEffectCommon::StartShadow() const
{
    shadowShader         .Bind();
    shadowUniforms       .Bind();
    shadowInputLayout    .Bind();
    shadowRasterizerState.Bind();
}

void DiffuseSolidBlockEffectCommon::EndShadow() const
{
    shadowShader         .Unbind();
    shadowUniforms       .Unbind();
    shadowInputLayout    .Unbind();
    shadowRasterizerState.Unbind();
}

const char *DiffuseSolidBlockEffect::GetName() const
{
    return name_.c_str();
}

bool DiffuseSolidBlockEffect::IsTransparent() const noexcept
{
    return false;
}

std::unique_ptr<ModelBuilder> DiffuseSolidBlockEffect::CreateModelBuilder(const Vec3i &globalSectionPosition) const
{
    return std::make_unique<Builder>(globalSectionPosition, this);
}

void DiffuseSolidBlockEffect::SetForwardRenderParams(const ForwardRenderParams &params) const
{
    common_->SetForwardRenderParams(params);
}

void DiffuseSolidBlockEffect::SetShadowRenderParams(const ShadowRenderParams &params) const
{
    common_->SetShadowRenderParams(params);
}

void DiffuseSolidBlockEffect::StartForward() const
{
    common_->StartForward(textureArray_.Get());
}

void DiffuseSolidBlockEffect::EndForward() const
{
    common_->EndForward();
}

void DiffuseSolidBlockEffect::StartShadow() const
{
    common_->StartShadow();
}

void DiffuseSolidBlockEffect::EndShadow() const
{
    common_->EndShadow();
}

void DiffuseSolidBlockEffect::Initialize(
    std::shared_ptr<DiffuseSolidBlockEffectCommon> common,
    ShaderResourceView textureArray, std::string name)
{
    common_ = std::move(common);
    textureArray_ = std::move(textureArray);
    name_ = std::move(name);
}

DiffuseSolidBlockEffectGenerator::DiffuseSolidBlockEffectGenerator(int textureSize, int maxArraySize)
{
    assert(textureSize > 0 && maxArraySize > 0);

    common_                   = std::make_shared<DiffuseSolidBlockEffectCommon>();
    textureSize_              = textureSize;
    maxArraySize_             = (std::min)(maxArraySize, D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION);
    nextEffectSemanticsIndex_ = 0;

    currentEffect_ = std::make_shared<DiffuseSolidBlockEffect>();
}

std::shared_ptr<DiffuseSolidBlockEffect> DiffuseSolidBlockEffectGenerator::GetEffectWithTextureSpaces(int textureCount)
{
    if(static_cast<int>(textureArrayData_.size()) + textureCount <= maxArraySize_)
    {
        return currentEffect_;
    }

    InitializeCurrentEffect();
    return currentEffect_;
}

int DiffuseSolidBlockEffectGenerator::AddTexture(const Vec4 *textureData)
{
    int ret = static_cast<int>(textureArrayData_.size());
    textureArrayData_.emplace_back(textureSize_, textureSize_, textureData);
    return ret;
}

void DiffuseSolidBlockEffectGenerator::Done()
{
    InitializeCurrentEffect();
}

void DiffuseSolidBlockEffectGenerator::InitializeCurrentEffect()
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
    textureDesc.MipLevels          = UINT(mipmapChains[0].chain_length());
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
        throw VRPGGameException("failed to create texture2d array for diffuse solid block effect");
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
        throw VRPGGameException("failed to create shader resource view of texture2d array of for diffuse solid block effect");
    }

    // 完成之前的effect的延迟初始化

    std::string name = "diffuse solid " + std::to_string(nextEffectSemanticsIndex_++);
    currentEffect_->Initialize(common_, ShaderResourceView(srv), std::move(name));
    BlockEffectManager::GetInstance().RegisterBlockEffect(currentEffect_);

    // 准备新的block effect

    textureArrayData_.clear();
    currentEffect_ = std::make_shared<DiffuseSolidBlockEffect>();
}

VRPG_GAME_END
