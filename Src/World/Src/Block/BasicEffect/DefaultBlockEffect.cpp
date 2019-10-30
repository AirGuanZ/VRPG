#include <agz/utility/file.h>

#include <VRPG/World/Block/BasicEffect/DefaultBlockEffect.h>

VRPG_WORLD_BEGIN

namespace
{
    class Model : public PartialSectionModel
    {
        const DefaultBlockEffect *effect_;
        VertexBuffer<DefaultBlockEffect::Vertex> vertexBuffer_;
        IndexBuffer<VertexIndex> indexBuffer_;
        
    public:

        Model(
            const DefaultBlockEffect *effect,
            VertexBuffer<DefaultBlockEffect::Vertex> vertexBuffer,
            IndexBuffer<VertexIndex> indexBuffer) noexcept
            : effect_(effect), vertexBuffer_(std::move(vertexBuffer)), indexBuffer_(std::move(indexBuffer))
        {

        }

        void Render(const Camera &camera) const override
        {
            vertexBuffer_.Bind(0);
            indexBuffer_.Bind();
            RenderState::DrawIndexed(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, indexBuffer_.GetIndexCount());
            indexBuffer_.Unbind();
            vertexBuffer_.Unbind(0);
        }

        const BlockEffect* GetBlockEffect() const noexcept override
        {
            return effect_;
        }
    };
}

DefaultBlockEffect::Builder::Builder(const DefaultBlockEffect *effect) noexcept
    : effect_(effect)
{
    assert(effect);
}

void DefaultBlockEffect::Builder::AddVertex(const Vertex &vertex)
{
    vertices_.push_back(vertex);
}

void DefaultBlockEffect::Builder::AddIndexedTriangle(uint16_t indexA, uint16_t indexB, uint16_t indexC)
{
    indices_.push_back(indexA);
    indices_.push_back(indexB);
    indices_.push_back(indexC);
}

std::shared_ptr<const PartialSectionModel> DefaultBlockEffect::Builder::Build() const
{
    if(vertices_.empty() || indices_.empty())
        return nullptr;

    VertexBuffer<Vertex> vertexBuffer;
    vertexBuffer.Initialize(UINT(vertices_.size()), false, vertices_.data());

    IndexBuffer<VertexIndex> indexBuffer;
    indexBuffer.Initialize(UINT(indices_.size()), false, indices_.data());

    return std::make_shared<Model>(effect_, std::move(vertexBuffer), std::move(indexBuffer));
}

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

std::unique_ptr<PartialSectionModelBuilder> DefaultBlockEffect::CreateModelBuilder() const
{
    return std::make_unique<Builder>(this);
}

void DefaultBlockEffect::SetRenderParams(const BlockRenderParams &params) const
{
    vsTransform_.SetValue({ params.camera->GetViewProjectionMatrix() });
    psSky_.SetValue({ params.skyLight, 0 });
}

VRPG_WORLD_END
