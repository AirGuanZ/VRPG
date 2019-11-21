#pragma once

#include <agz/utility/texture.h>

#include <VRPG/Game/Block/BlockEffect.h>
#include <VRPG/Game/Misc/CascadeShadowMapping.h>

VRPG_GAME_BEGIN

/**
 * @brief 所有半透明方块只能走这一个Effect
 */
class TransparentBlockEffect : public BlockEffect
{
    friend class TransparentBlockEffectGenerator;

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
        Mat4 VP;
    };

    struct PS_PerFrame
    {
        Vec3 skyLight;
        float pad = 0;
    };

    class Builder : public ModelBuilder
    {
    public:

        struct FaceIndexRange
        {
            Vec3 position;
            VertexIndex startIndex = 0;
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

    const char *GetName() const override;

    bool IsTransparent() const noexcept override;

    void StartForward() const override;

    void EndForward() const override;

    std::unique_ptr<ModelBuilder> CreateModelBuilder(const Vec3i &globalSectionPosition) const override;

    void SetForwardRenderParams(const BlockForwardRenderParams &params) const override;

private:

    void Initialize(int textureSize, const std::vector<agz::texture::texture2d_t<Vec4>> &textureArrayData);

    Shader<SS_VS, SS_PS>                 shader_;
    UniformManager<SS_VS, SS_PS>         uniforms_;
    InputLayout                          inputLayout_;
    mutable ConstantBuffer<VS_Transform> vsTransform_;
    mutable ConstantBuffer<PS_PerFrame>  psPerFrame_;
    BlendState                           blendState_;
    DepthState                           depthState_;

    std::unique_ptr<ForwardShadowMapping> forwardShadowMapping_;
};

class TransparentBlockEffectGenerator : public agz::misc::uncopyable_t
{
public:

    explicit TransparentBlockEffectGenerator(int textureSize);

    std::shared_ptr<TransparentBlockEffect> GetEffectWithTextureSpaces();

    int AddTexture(const Vec4 *textureData);

    void Done();

private:

    std::vector<agz::texture::texture2d_t<Vec4>> textureArrayData_;
    int textureSize_;

    std::shared_ptr<TransparentBlockEffect> currentEffect_;
};

VRPG_GAME_END
