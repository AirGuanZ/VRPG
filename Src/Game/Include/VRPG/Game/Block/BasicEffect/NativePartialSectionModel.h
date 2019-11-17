#pragma once

#include <VRPG/Game/Block/BlockEffect.h>
#include <VRPG/Game/Chunk/ChunkModel.h>

VRPG_GAME_BEGIN

/**
 * @brief 由一块vertex buffer和index buffer构成的partial section model
 */
template<typename Effect>
class NativePartialSectionModel : public PartialSectionModel
{
public:

    NativePartialSectionModel(
        const Vec3i &globalSectionPosition, const Effect *effect,
        VertexBuffer<typename Effect::Vertex> vertexBuffer,
        IndexBuffer<VertexIndex> indexBuffer) noexcept
        : PartialSectionModel(globalSectionPosition), effect_(effect),
          vertexBuffer_(std::move(vertexBuffer)), indexBuffer_(std::move(indexBuffer))
    {

    }

    void Render(const Camera &camera) const override
    {
        this->RenderShadow();
    }

    void RenderShadow() const override
    {
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

private:

    static_assert(std::is_base_of_v<BlockEffect, Effect>);

    const Effect                         *effect_;
    VertexBuffer<typename Effect::Vertex> vertexBuffer_;
    IndexBuffer<VertexIndex>              indexBuffer_;
};

/**
 * @brief 用于构建NativePartialSectionModel的model builder
 */
template<typename Effect>
class NativePartialSectionModelBuilder : public ModelBuilder
{
public:

    explicit NativePartialSectionModelBuilder(const Vec3i &globalSectionPosition, const Effect *effect)
        : globalSectionPosition_(globalSectionPosition), effect_(effect)
    {
        
    }

    void AddVertex(const typename Effect::Vertex &vertex)
    {
        vertices_.push_back(vertex);
    }

    void AddIndexedTriangle(uint16_t indexA, uint16_t indexB, uint16_t indexC)
    {
        indices_.push_back(indexA);
        indices_.push_back(indexB);
        indices_.push_back(indexC);
    }

    size_t GetVertexCount() const noexcept { return vertices_.size(); }

    std::shared_ptr<const PartialSectionModel> Build() override
    {
        if(vertices_.empty() || indices_.empty())
        {
            return nullptr;
        }

        VertexBuffer<typename Effect::Vertex> vertexBuffer;
        vertexBuffer.Initialize(UINT(vertices_.size()), false, vertices_.data());

        IndexBuffer<VertexIndex> indexBuffer;
        indexBuffer.Initialize(UINT(indices_.size()), false, indices_.data());

        return std::make_shared<NativePartialSectionModel<Effect>>(
            globalSectionPosition_, effect_, std::move(vertexBuffer), std::move(indexBuffer));
    }

private:

    static_assert(std::is_base_of_v<BlockEffect, Effect>);

    Vec3i globalSectionPosition_;

    std::vector<typename Effect::Vertex> vertices_;
    std::vector<VertexIndex>             indices_;
    const Effect                        *effect_;
};

VRPG_GAME_END
