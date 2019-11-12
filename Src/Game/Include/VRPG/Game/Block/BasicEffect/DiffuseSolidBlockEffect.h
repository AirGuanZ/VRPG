#pragma once

#include <optional>

#include <agz/utility/texture.h>

#include <VRPG/Game/Block/BasicEffect/NativePartialSectionModel.h>
#include <VRPG/Game/Block/BasicEffect/ShadowMapEffect.h>
#include <VRPG/Game/Block/BlockEffect.h>

VRPG_GAME_BEGIN

class DiffuseSolidBlockEffect;

/**
 * 一个DiffuseSolidBlockEffect中存有一个表示Diffuse Albedo的Texture Array，
 * 而单个Texture Array未必能存下所有需要的Texture，所以可能会有多个DiffuseSolidBlockEffect实例。
 * 这些实例共享Shader、Constant Buffer等，只有Texture Array不同
 */
class DiffuseSolidBlockEffectGenerator
{
public:

    struct Vertex
    {
        Vec3 position;
        Vec2 texCoord;
        uint32_t texIndex = 0;
        Vec4 brightness;
    };

    struct Forward_VS_Transform
    {
        Mat4 shadowVP;
        Mat4 VP;
    };

    struct Forward_PS_PerFrame
    {
        Vec3 skyLight;
        float shadowScale = 1;
    };

    struct Shadow_VS_Transform
    {
        Mat4 VP;
    };

    /**
     * @brief 由多个DiffuseBlockEffect实例所共享的数据
     */
    class CommonProperties
    {
        void InitializeForward();

    public:

        CommonProperties();

        Shader<SS_VS, SS_PS> forwardShader_;
        UniformManager<SS_VS, SS_PS> forwardUniforms_;
        InputLayout forwardInputLayout_;
        ConstantBuffer<Forward_VS_Transform> forwardVSTransform_;
        ConstantBuffer<Forward_PS_PerFrame> forwardPSPerFrame_;
        ShaderResourceSlot<SS_PS> *forwardDiffuseTextureSlot_;
        ShaderResourceSlot<SS_PS> *forwardShadowMapSlot_;

        ShadowMapEffect shadowMap;
    };

    DiffuseSolidBlockEffectGenerator(int textureSize, int expectedArraySize);

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
    void InitializeEffect(DiffuseSolidBlockEffect &effect);

private:

    std::shared_ptr<CommonProperties> commonProperties_;

    std::vector<agz::texture::texture2d_t<Vec4>> textureArrayData_;
    int textureSize_;
    int maxArraySize_;

    int generatedEffectCount_;
};

class DiffuseSolidBlockEffect : public BlockEffect
{
public:

    using Generator = DiffuseSolidBlockEffectGenerator;
    using Vertex = Generator::Vertex;

    using Builder = NativePartialSectionModelBuilder<DiffuseSolidBlockEffect>;

    void Initialize(
        std::shared_ptr<Generator::CommonProperties> commonProperties,
        ShaderResourceView textureArray, int semanticsIndex);

    const char *GetName() const override;

    bool IsTransparent() const noexcept override;

    void StartForward() const override;

    void EndForward() const override;

    void StartShadow() const override;

    void EndShadow() const override;

    std::unique_ptr<PartialSectionModelBuilder> CreateModelBuilder(const Vec3i &globalSectionPosition) const override;

    void SetForwardRenderParams(const BlockForwardRenderParams &params) const override;

    void SetShadowRenderParams(const BlockShadowRenderParams &params) const override;

private:

    std::shared_ptr<Generator::CommonProperties> commonProperties_;

    ShaderResourceView textureArray_;
    std::string name_;
};

VRPG_GAME_END
