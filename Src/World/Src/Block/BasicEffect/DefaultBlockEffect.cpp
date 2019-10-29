#include <agz/utility/file.h>

#include <VRPG/World/Block/BasicEffect/DefaultBlockEffect.h>

VRPG_WORLD_BEGIN

namespace
{
    class Model : public PartialSectionModel
    {
        const DefaultBlockEffect *blockEffect_;
        VertexBuffer<DefaultBlockEffect::Vertex> vertexBuffer_;
        
    public:

        Model(const DefaultBlockEffect *effect, VertexBuffer<DefaultBlockEffect::Vertex> vertexBuffer)
            : blockEffect_(effect), vertexBuffer_(std::move(vertexBuffer))
        {

        }

        void Render(const Camera &camera) const override
        {
            blockEffect_->_setVSTransform({ camera.GetViewProjectionMatrix() });

            vertexBuffer_.Bind(0);
            RenderState::Draw(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, vertexBuffer_.GetVertexCount());
            vertexBuffer_.Unbind(0);
        }

        const BlockEffect* GetBlockEffect() const noexcept override
        {
            return blockEffect_;
        }
    };
}

DefaultBlockEffect::Builder::Builder(const DefaultBlockEffect *effect) noexcept
    : effect_(effect)
{
    assert(effect);
}

void DefaultBlockEffect::Builder::AddTriangle(const Vertex &a, const Vertex &b, const Vertex &c)
{
    vertices_.reserve(vertices_.size() + 3);
    vertices_.push_back(a);
    vertices_.push_back(b);
    vertices_.push_back(c);
}

std::shared_ptr<const PartialSectionModel> DefaultBlockEffect::Builder::Build() const
{
    if(vertices_.empty())
        return nullptr;
    VertexBuffer<Vertex> vertexBuffer;
    vertexBuffer.Initialize(UINT(vertices_.size()), false, vertices_.data());
    return std::make_shared<Model>(effect_, std::move(vertexBuffer));
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

void DefaultBlockEffect::SetSkyLight(const Vec3 &light) const
{
    psSky_.SetValue({ light, 0 });
}

void DefaultBlockEffect::_setVSTransform(const VS_Transform &transform) const
{
    vsTransform_.SetValue(transform);
}

VRPG_WORLD_END
