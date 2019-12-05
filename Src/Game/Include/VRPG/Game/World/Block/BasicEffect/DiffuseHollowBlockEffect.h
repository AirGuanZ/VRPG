#pragma once

#include <agz/utility/texture.h>

#include <VRPG/Game/Misc/CascadeShadowMapping.h>
#include <VRPG/Game/World/Block/BasicEffect/NativePartialSectionModel.h>
#include <VRPG/Game/World/Block/BlockEffect.h>

VRPG_GAME_BEGIN

class DiffuseHollowBlockEffectCommon
{
    struct Forward_VS_Transform
    {
        Mat4 VP;
    };

    struct Forward_PS_PerFrame
    {
        Vec3 skyLight;
        float pad = 0;
    };

    struct Shadow_VS_Transform
    {
        Mat4 VP;
    };

    Shader<SS_VS, SS_PS>                  forwardShader_;
    UniformManager<SS_VS, SS_PS>          forwardUniforms_;
    InputLayout                           forwardInputLayout_;
    ConstantBuffer<Forward_VS_Transform>  forwardVSTransform_;
    ConstantBuffer<Forward_PS_PerFrame>   forwardPSPerFrame_;
    ShaderResourceSlot<SS_PS>            *forwardDiffuseTextureSlot_;
    RasterizerState                       forwardRasterizerState_;
    std::unique_ptr<ForwardShadowMapping> forwardShadowMapping_;

    Shader<SS_VS, SS_PS>                shadowShader_;
    UniformManager<SS_VS, SS_PS>        shadowUniforms_;
    InputLayout                         shadowInputLayout_;
    ConstantBuffer<Shadow_VS_Transform> shadowVSTransform_;
    ShaderResourceSlot<SS_PS>          *shadowDiffuseTextureSlot_;
    RasterizerState                     shadowRasterizerState_;

    void InitializeForward();

    void InitializeShadow();

public:

    DiffuseHollowBlockEffectCommon();

    void SetForwardRenderParams(const ForwardRenderParams &params);

    void SetShadowRenderParams(const ShadowRenderParams &params);

    void StartForward(ID3D11ShaderResourceView *textureArray);

    void EndForward();

    void StartShadow(ID3D11ShaderResourceView *textureArray);

    void EndShadow();
};

class DiffuseHollowBlockEffect : public BlockEffect
{
    friend class DiffuseHollowBlockEffectGenerator;

public:

    struct Vertex
    {
        Vec3 position;
        Vec2 texCoord;
        Vec3 normal;
        Vec4 brightness;
        uint32_t texIndex = 0;
    };

    using Builder = NativePartialSectionModelBuilder<DiffuseHollowBlockEffect>;

    const char *GetName() const override;

    bool IsTransparent() const noexcept override;

    void StartForward() const override;

    void EndForward() const override;

    void StartShadow() const override;

    void EndShadow() const override;

    std::unique_ptr<ModelBuilder> CreateModelBuilder(const Vec3i &globalSectionPosition) const override;

    void SetForwardRenderParams(const ForwardRenderParams &params) const override;

    void SetShadowRenderParams(const ShadowRenderParams &params) const override;

private:

    void Initialize(
        std::shared_ptr<DiffuseHollowBlockEffectCommon> commonProperties,
        ShaderResourceView textureArray, std::string name);

    std::shared_ptr<DiffuseHollowBlockEffectCommon> common_;
    ShaderResourceView textureArray_;
    std::string name_;
};

class DiffuseHollowBlockEffectGenerator : public agz::misc::uncopyable_t
{
public:

    DiffuseHollowBlockEffectGenerator(int textureSize, int maxArraySize);

    std::shared_ptr<DiffuseHollowBlockEffect> GetEffectWithTextureSpaces(int textureCount);

    int AddTexture(const Vec4 *textureData);

    void Done();

private:

    void InitializeCurrentEffect();

    std::shared_ptr<DiffuseHollowBlockEffectCommon> common_;

    std::vector<agz::texture::texture2d_t<Vec4>> textureArrayData_;
    int textureSize_;
    int maxArraySize_;

    int nextEffectSemanticsIndex_;
    std::shared_ptr<DiffuseHollowBlockEffect> currentEffect_;
};

VRPG_GAME_END
