#include <agz/utility/file.h>

#include <VRPG/World/Block/BasicEffect/DiffuseBlockEffect.h>

VRPG_WORLD_BEGIN

DiffuseBlockEffectGenerator::CommonProperties::CommonProperties()
{
    std::string vertexShaderSource = agz::file::read_txt_file("Asset/World/Shader/BlockEffect/DiffuseVertex.hlsl");
    std::string pixelShaderSource  = agz::file::read_txt_file("Asset/World/Shader/BlockEffect/DiffusePixel.hlsl");
    shader_.InitializeStage<SS_VS>(vertexShaderSource);
    shader_.InitializeStage<SS_PS>(pixelShaderSource);
    if(!shader_.IsAllStagesAvailable())
        throw VRPGWorldException("failed to initialize diffuse block effect shader");

    uniforms_ = shader_.CreateUniformManager();

    inputLayout_ = InputLayoutBuilder
        ("POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT,    ByteOffset(&Vertex::position))
        ("TEXCOORD",   0, DXGI_FORMAT_R32G32_FLOAT,       ByteOffset(&Vertex::texCoord))
        ("TEXINDEX",   0, DXGI_FORMAT_R32_UINT,           ByteOffset(&Vertex::texIndex))
        ("BRIGHTNESS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, ByteOffset(&Vertex::brightness))
        .Build(shader_.GetVertexShaderByteCode());

    vsTransform_.Initialize(true, nullptr);
    psSky_.Initialize(true, nullptr);

    uniforms_.GetConstantBufferSlot<SS_VS>("Transform")->SetBuffer(vsTransform_);
    uniforms_.GetConstantBufferSlot<SS_PS>("Sky")->SetBuffer(psSky_);

    Sampler sampler;
    sampler.Initialize(D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR);
    uniforms_.GetSamplerSlot<SS_PS>("DiffuseSampler")->SetSampler(sampler);

    diffuseTextureSlot_ = uniforms_.GetShaderResourceSlot<SS_PS>("DiffuseTexture");
    if(!diffuseTextureSlot_)
        throw VRPGWorldException("shader resource slot not found in diffuse block effect shader: DiffuseTexture");
}

DiffuseBlockEffectGenerator::DiffuseBlockEffectGenerator(int textureSize, int expectedArraySize)
{
    assert(textureSize > 0 && expectedArraySize > 0);
    commonProperties_ = std::make_shared<CommonProperties>();
    textureSize_ = textureSize;
    maxArraySize_ = (std::min)(expectedArraySize, D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION);
    generatedEffectCount_ = 0;
}

bool DiffuseBlockEffectGenerator::IsFull() const noexcept
{
    return int(textureArrayData_.size()) >= maxArraySize_;
}

bool DiffuseBlockEffectGenerator::IsEmpty() const noexcept
{
    return textureArrayData_.empty();
}

bool DiffuseBlockEffectGenerator::HasEnoughSpaceFor(int arrayDataCount) const noexcept
{
    return int(textureArrayData_.size()) + arrayDataCount <= maxArraySize_;
}

int DiffuseBlockEffectGenerator::AddTexture(const Vec3 *data)
{
    assert(!IsFull());
    int ret = int(textureArrayData_.size());
    textureArrayData_.emplace_back(textureSize_, textureSize_, data);
    return ret;
}

void DiffuseBlockEffectGenerator::InitializeEffect(DiffuseBlockEffect &effect)
{
    assert(!textureArrayData_.empty());

    // 生成mipmap chain

    std::vector<agz::texture::mipmap_chain_t<Vec3>> mipmapChains(textureArrayData_.size());
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
    textureDesc.Format             = DXGI_FORMAT_R32G32B32_FLOAT;
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
            initData.SysMemPitch      = mipmapData.width() * sizeof(Vec3);
            initData.SysMemSlicePitch = 0;
        }
    }

    ComPtr<ID3D11Texture2D> texture = Base::D3D::CreateTexture2D(textureDesc, initDataArr.data());
    if(!texture)
        throw VRPGWorldException("failed to create texture2d array for diffuse block effect");

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format                         = DXGI_FORMAT_R32G32B32_FLOAT;
    srvDesc.ViewDimension                  = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    srvDesc.Texture2DArray.MostDetailedMip = 0;
    srvDesc.Texture2DArray.MipLevels       = -1;
    srvDesc.Texture2DArray.FirstArraySlice = 0;
    srvDesc.Texture2DArray.ArraySize       = UINT(textureArrayData_.size());

    ComPtr<ID3D11ShaderResourceView> srv = Base::D3D::CreateShaderResourceView(srvDesc, texture.Get());
    if(!srv)
        throw VRPGWorldException("failed to create shader resource view of texture2d array of for diffuse block effect");

	effect.Initialize(commonProperties_, ShaderResourceView(srv), generatedEffectCount_++);
}

void DiffuseBlockEffect::Initialize(
    std::shared_ptr<Generator::CommonProperties> commonProperties,
    ShaderResourceView textureArray, int semanticsIndex)
{
    commonProperties_ = std::move(commonProperties);
    textureArray_ = std::move(textureArray);
    name_ = "diffuse_" + std::to_string(semanticsIndex);
}

const char *DiffuseBlockEffect::GetName() const
{
    return name_.c_str();
}

void DiffuseBlockEffect::Bind() const
{
    commonProperties_->diffuseTextureSlot_->SetShaderResourceView(textureArray_);
    commonProperties_->shader_.Bind();
    commonProperties_->uniforms_.Bind();
    commonProperties_->inputLayout_.Bind();
}

void DiffuseBlockEffect::Unbind() const
{
    commonProperties_->shader_.Unbind();
    commonProperties_->uniforms_.Unbind();
    commonProperties_->inputLayout_.Unbind();
}

std::unique_ptr<PartialSectionModelBuilder> DiffuseBlockEffect::CreateModelBuilder() const
{
    return std::make_unique<Builder>(this);
}

void DiffuseBlockEffect::SetRenderParams(const BlockRenderParams &params) const
{
    commonProperties_->vsTransform_.SetValue({ params.camera->GetViewProjectionMatrix() });
    commonProperties_->psSky_.SetValue({ params.skyLight, 0 });
}

VRPG_WORLD_END
