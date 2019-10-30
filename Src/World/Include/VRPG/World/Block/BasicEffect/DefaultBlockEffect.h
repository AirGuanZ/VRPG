#pragma once

#include <vector>

#include <VRPG/World/Block/BlockEffect.h>
#include <VRPG/World/Chunk/ChunkModel.h>

VRPG_WORLD_BEGIN

class DefaultBlockEffect : public BlockEffect
{
public:

    struct Vertex
    {
        Vec3 position;
        Vec4 brightness;
    };

    struct VS_Transform
    {
        Mat4 WVP;
    };

    struct PS_Sky
    {
        Vec3 skylight;
        float pad = 0;
    };

    class Builder : public PartialSectionModelBuilder
    {
        std::vector<Vertex> vertices_;
        std::vector<VertexIndex> indices_;
        const DefaultBlockEffect *effect_;

    public:

        explicit Builder(const DefaultBlockEffect *effect) noexcept;

        void AddVertex(const Vertex &vertex);

        void AddIndexedTriangle(uint16_t indexA, uint16_t indexB, uint16_t indexC);

        size_t GetVertexCount() const noexcept { return vertices_.size(); }

        std::shared_ptr<const PartialSectionModel> Build() const override;
    };

    DefaultBlockEffect();

    const char *GetName() const override;

    void Bind() const override;

    void Unbind() const override;

    std::unique_ptr<PartialSectionModelBuilder> CreateModelBuilder() const override;

    void SetRenderParams(const BlockRenderParams &params) const override;

private:

    Shader<SS_VS, SS_PS> shader_;
    UniformManager<SS_VS, SS_PS> uniforms_;

    InputLayout inputLayout_;

    mutable ConstantBuffer<VS_Transform> vsTransform_;
    mutable ConstantBuffer<PS_Sky> psSky_;
};

VRPG_WORLD_END
