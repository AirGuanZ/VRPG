#pragma once

#include <agz/utility/texture.h>

#include <VRPG/World/Block/BlockEffect.h>

VRPG_WORLD_BEGIN

/**
 * @brief 所有半透明方块只能走这一个Effect
 */
class TransparentBlockEffect : public BlockEffect
{
public:

    struct Vertex
    {
        Vec3 position;
        Vec2 texCoord;
        uint32_t texIndex = 0;
        Vec4 brightness;
    };

    struct VS_Transform
    {
        Mat4 WVP;
    };

    struct PS_Sky
    {
        Vec3 skyLight;
        float pad = 0;
    };

    class Builder : public PartialSectionModelBuilder
    {
    public:

        struct FaceIndexRange
        {
            Vec3 position;
            VertexIndex startIndex;
        };

        explicit Builder(const Vec3i &globalSectionPosition, const TransparentBlockEffect *effect);

        void AddVertex(const Vertex &vertex);

        void AddIndexedTriangle(VertexIndex indexA, VertexIndex indexB, VertexIndex indexC);

        void AddFaceIndexRange(const Vec3 &facePosition, VertexIndex startIndex);

        size_t GetVertexCount() const noexcept { return vertices_.size(); }

        size_t GetIndexCount() const noexcept { return indices_.size(); }

        std::shared_ptr<const PartialSectionModel> Build() override;

    private:

        Vec3i globalSectionPosition_;

        std::vector<Vertex> vertices_;
        std::vector<VertexIndex> indices_;
        std::vector<FaceIndexRange> faces_;

        const TransparentBlockEffect *effect_;
    };

    int AddTexture(agz::texture::texture2d_t<Vec4> textureData);

    void Initialize();

    const char *GetName() const override;

    bool IsTransparent() const noexcept override;

    void Bind() const override;

    void Unbind() const override;

    std::unique_ptr<PartialSectionModelBuilder> CreateModelBuilder(const Vec3i &globalSectionPosition) const override;

    void SetRenderParams(const BlockRenderParams &params) const override;

private:

    Shader<SS_VS, SS_PS> shader_;
    UniformManager<SS_VS, SS_PS> uniforms_;

    InputLayout inputLayout_;

    mutable ConstantBuffer<VS_Transform> vsTransform_;
    mutable ConstantBuffer<PS_Sky> psSky_;

    BlendState blendState_;
    DepthState depthState_;

    std::vector<agz::texture::texture2d_t<Vec4>> textureDataArray_;
    int textureSize_ = 0;
};

VRPG_WORLD_END
