#pragma once

#include <optional>

#include <agz/utility/texture.h>

#include <VRPG/Game/Block/BasicEffect/NativePartialSectionModel.h>
#include <VRPG/Game/Block/BlockEffect.h>

VRPG_GAME_BEGIN

class DiffuseHollowBlockEffect;

/**
 * 一个DiffuseHollowBlockEffect中存有一个表示Diffuse Albedo的Texture Array，
 * 而单个Texture Array未必能存下所有需要的Texture，所以可能会有多个DiffuseHollowBlockEffect实例。
 * 这些实例共享Shader、Constant Buffer等，只有Texture Array不同
 */
class DiffuseHollowBlockEffectGenerator
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

    /**
     * @brief 由多个DiffuseHollowBlockEffect实例所共享的数据
     */
    struct CommonProperties
    {
        CommonProperties();

        Shader<SS_VS, SS_PS> shader_;
        UniformManager<SS_VS, SS_PS> uniforms_;

        InputLayout inputLayout_;

        ConstantBuffer<VS_Transform> vsTransform_;
        ConstantBuffer<PS_Sky> psSky_;

        ShaderResourceSlot<SS_PS> *diffuseTextureSlot_;

        RasterizerState rasterizerState_;
    };

    DiffuseHollowBlockEffectGenerator(int textureSize, int expectedArraySize);

    /**
     * @brief array是否为空
     */
    bool IsEmpty() const noexcept;

    /**
     * @brief array中是否还能再增加arrayDataCount个texture data
     */
    bool HasEnoughSpaceFor(int arrayDataCount) const noexcept;

    /**
     * @brief 向texture array中添加一个textureSize^2大小的texture data
     *
     * 返回新添加的texture data在texture array中的下标
     */
    int AddTexture(const Vec4 *data);

    /**
     * @brief 初始化给定的effect
     */
    void InitializeEffect(DiffuseHollowBlockEffect &effect);

private:

    std::shared_ptr<CommonProperties> commonProperties_;

    std::vector<agz::texture::texture2d_t<Vec4>> textureArrayData_;
    int textureSize_;
    int maxArraySize_;

    int generatedEffectCount_;
};

class DiffuseHollowBlockEffect : public BlockEffect
{
public:

    using Generator = DiffuseHollowBlockEffectGenerator;
    using Vertex = Generator::Vertex;

    using Builder = NativePartialSectionModelBuilder<DiffuseHollowBlockEffect>;

    void Initialize(
        std::shared_ptr<Generator::CommonProperties> commonProperties,
        ShaderResourceView textureArray, int semanticsIndex);

    const char *GetName() const override;

    bool IsTransparent() const noexcept override;

    void Bind() const override;

    void Unbind() const override;

    std::unique_ptr<PartialSectionModelBuilder> CreateModelBuilder(const Vec3i &globalSectionPosition) const override;

    void SetRenderParams(const BlockRenderParams &params) const override;

private:

    std::shared_ptr<Generator::CommonProperties> commonProperties_;

    ShaderResourceView textureArray_;
    std::string name_;
};

VRPG_GAME_END
