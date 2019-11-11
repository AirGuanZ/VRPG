#include <agz/utility/file.h>

#include <VRPG/Game/Block/BasicEffect/TransparentBlockEffect.h>

VRPG_GAME_BEGIN

namespace
{
    class Model : public PartialSectionModel
    {
        const TransparentBlockEffect *effect_;

        VertexBuffer<TransparentBlockEffect::Vertex> vertexBuffer_;

        std::vector<VertexIndex> originalIndices_;
        mutable std::vector<TransparentBlockEffect::Builder::FaceIndexRange> blocks_;
        mutable IndexBuffer<VertexIndex> indexBuffer_;
        mutable std::vector<VertexIndex> tempIndices_;

    public:

        Model(
            const Vec3i &globalSectionPosition, const TransparentBlockEffect *effect,
            VertexBuffer<TransparentBlockEffect::Vertex> vertexBuffer,
            IndexBuffer<VertexIndex> indexBuffer,
            std::vector<VertexIndex> originalIndices,
            std::vector<TransparentBlockEffect::Builder::FaceIndexRange> blocks)
            : PartialSectionModel(globalSectionPosition), effect_(effect),
              vertexBuffer_(std::move(vertexBuffer)), originalIndices_(std::move(originalIndices)),
              blocks_(std::move(blocks)), indexBuffer_(std::move(indexBuffer))
        {
            tempIndices_.resize(originalIndices_.size());
        }

        void Render(const Camera &camera) const override
        {
            std::sort(blocks_.begin(), blocks_.end(),
                [eye = camera.GetPosition()](auto &lhs, auto &rhs)
            {
                return (lhs.position - eye).length_square() > (rhs.position - eye).length_square();
            });

            VertexIndex indexCount = 0;
            for(auto &block : blocks_)
            {
                VertexIndex startIndex = block.startIndex;
                for(int i = 0; i < 12; ++i)
                    tempIndices_[indexCount++] = originalIndices_[startIndex++];
            }
            indexBuffer_.SetValue(tempIndices_.data());

            vertexBuffer_.Bind(0);
            indexBuffer_.Bind();
            RenderState::DrawIndexed(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, indexBuffer_.GetIndexCount());
            indexBuffer_.Unbind();
            vertexBuffer_.Unbind(0);
        }

        const BlockEffect *GetBlockEffect() const noexcept override
        {
            return effect_;
        }
    };
}

TransparentBlockEffect::Builder::Builder(const Vec3i &globalSectionPosition, const TransparentBlockEffect *effect)
    : globalSectionPosition_(globalSectionPosition), effect_(effect)
{
    
}

void TransparentBlockEffect::Builder::AddVertex(const Vertex &vertex)
{
    vertices_.push_back(vertex);
}

void TransparentBlockEffect::Builder::AddIndexedTriangle(VertexIndex indexA, VertexIndex indexB, VertexIndex indexC)
{
    indices_.push_back(indexA);
    indices_.push_back(indexB);
    indices_.push_back(indexC);
}

void TransparentBlockEffect::Builder::AddFaceIndexRange(const Vec3 &blockInSection, VertexIndex startIndex)
{
    faces_.push_back({ blockInSection, startIndex });
}

std::shared_ptr<const PartialSectionModel> TransparentBlockEffect::Builder::Build()
{
    if(indices_.empty())
        return nullptr;

    VertexBuffer<Vertex> vertexBuffer;
    vertexBuffer.Initialize(UINT(vertices_.size()), false, vertices_.data());

    IndexBuffer<VertexIndex> indexBuffer;
    indexBuffer.Initialize(UINT(indices_.size()), true, nullptr);

    return std::make_shared<Model>(
        globalSectionPosition_, effect_, std::move(vertexBuffer), std::move(indexBuffer), std::move(indices_), std::move(faces_));
}

int TransparentBlockEffect::AddTexture(agz::texture::texture2d_t<Vec4> textureData)
{
    assert(textureData.width() == textureData.height());
    if(!textureSize_)
        textureSize_ = textureData.width();
    else if(textureSize_ != textureData.width())
    {
        throw VRPGWorldException(
            "invalid texture data size. expected: " + std::to_string(textureSize_) + 
            ", actual: " + std::to_string(textureData.width()));
    }

    int ret = int(textureDataArray_.size());
    textureDataArray_.push_back(std::move(textureData));
    return ret;
}

void TransparentBlockEffect::Initialize()
{
    assert(!textureDataArray_.empty());

    std::string vertexShaderSource = agz::file::read_txt_file("Asset/World/Shader/BlockEffect/TransparentVertex.hlsl");
    std::string pixelShaderSource  = agz::file::read_txt_file("Asset/World/Shader/BlockEffect/TransparentPixel.hlsl");
    shader_.InitializeStage<SS_VS>(vertexShaderSource);
    shader_.InitializeStage<SS_PS>(pixelShaderSource);
    if(!shader_.IsAllStagesAvailable())
        throw VRPGWorldException("failed to initialize transparent box block effect");

    uniforms_ = shader_.CreateUniformManager();
    
    inputLayout_ = InputLayoutBuilder
        ("POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT,    offsetof(Vertex, position))
        ("TEXCOORD",   0, DXGI_FORMAT_R32G32_FLOAT,       offsetof(Vertex, texCoord))
        ("TEXINDEX",   0, DXGI_FORMAT_R32_UINT,           offsetof(Vertex, texIndex))
        ("BRIGHTNESS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, offsetof(Vertex, brightness))
        .Build(shader_.GetVertexShaderByteCode());

    vsTransform_.Initialize(true, nullptr);
    psSky_.Initialize(true, nullptr);

    uniforms_.GetConstantBufferSlot<SS_VS>("Transform")->SetBuffer(vsTransform_);
    uniforms_.GetConstantBufferSlot<SS_PS>("Sky")->SetBuffer(psSky_);

    Sampler sampler;
    sampler.Initialize(D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR);
    uniforms_.GetSamplerSlot<SS_PS>("TransparentSampler")->SetSampler(sampler);

    blendState_ = BlendStateBuilder()
        .Set(0, true,
            D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD,
            D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD)
        .Build();
    depthState_.Initialize(true, D3D11_DEPTH_WRITE_MASK_ZERO);

    std::vector<agz::texture::mipmap_chain_t<Vec4>> mipmapChains(textureDataArray_.size());
    for(size_t i = 0; i < textureDataArray_.size(); ++i)
    {
        auto &texData = textureDataArray_[i];
        mipmapChains[i].generate(texData);
    }

    D3D11_TEXTURE2D_DESC texDesc;
    texDesc.Width              = UINT(textureSize_);
    texDesc.Height             = UINT(textureSize_);
    texDesc.MipLevels          = UINT(mipmapChains[0].chain_length());
    texDesc.ArraySize          = UINT(textureDataArray_.size());
    texDesc.Format             = DXGI_FORMAT_R32G32B32A32_FLOAT;
    texDesc.SampleDesc.Count   = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage              = D3D11_USAGE_IMMUTABLE;
    texDesc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags     = 0;
    texDesc.MiscFlags          = 0;

    std::vector<D3D11_SUBRESOURCE_DATA> initDataArr(textureDataArray_.size() * mipmapChains[0].chain_length());
    int initDataArrIndex = 0;
    for(size_t i = 0; i < textureDataArray_.size(); ++i)
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

    ComPtr<ID3D11Texture2D> texture = Base::D3D::CreateTexture2D(texDesc, initDataArr.data());
    if(!texture)
        throw VRPGWorldException("failed to create texture2d array for transparent block effect");
    
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format                         = DXGI_FORMAT_R32G32B32A32_FLOAT;
    srvDesc.ViewDimension                  = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    srvDesc.Texture2DArray.MostDetailedMip = 0;
    srvDesc.Texture2DArray.MipLevels       = -1;
    srvDesc.Texture2DArray.FirstArraySlice = 0;
    srvDesc.Texture2DArray.ArraySize       = UINT(textureDataArray_.size());

    ComPtr<ID3D11ShaderResourceView> srv = Base::D3D::CreateShaderResourceView(srvDesc, texture.Get());
    if(!srv)
        throw VRPGWorldException("failed to create shader resource view of texture2d array of for transparent block effect");

    uniforms_.GetShaderResourceSlot<SS_PS>("TransparentTexture")->SetShaderResourceView(ShaderResourceView(srv));

    decltype(textureDataArray_) tTexDataArr;
    textureDataArray_.swap(tTexDataArr);
}

const char *TransparentBlockEffect::GetName() const
{
    return "transparent";
}

bool TransparentBlockEffect::IsTransparent() const noexcept
{
    return true;
}

void TransparentBlockEffect::Bind() const
{
    shader_.Bind();
    uniforms_.Bind();
    inputLayout_.Bind();
    blendState_.Bind();
    depthState_.Bind();
}

void TransparentBlockEffect::Unbind() const
{
    depthState_.Unbind();
    blendState_.Unbind();
    inputLayout_.Unbind();
    uniforms_.Unbind();
    shader_.Unbind();
}

std::unique_ptr<PartialSectionModelBuilder> TransparentBlockEffect::CreateModelBuilder(const Vec3i &globalSectionPosition) const
{
    return std::make_unique<Builder>(globalSectionPosition, this);
}

void TransparentBlockEffect::SetRenderParams(const BlockRenderParams &params) const
{
    vsTransform_.SetValue({ params.camera->GetViewProjectionMatrix() });
    psSky_.SetValue({ params.skyLight, 0 });
}

VRPG_GAME_END
