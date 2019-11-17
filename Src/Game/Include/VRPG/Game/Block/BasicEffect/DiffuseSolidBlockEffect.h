#pragma once

#include <agz/utility/texture.h>

#include <VRPG/Game/Block/BasicEffect/NativePartialSectionModel.h>
#include <VRPG/Game/Block/BasicEffect/ShadowMappingUtility.h>
#include <VRPG/Game/Block/BlockEffect.h>

VRPG_GAME_BEGIN

class DiffuseSolidBlockEffectCommon
{
    void InitializeForward();
    void InitializeShadow();

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

    Shader<SS_VS, SS_PS>                 forwardShader;
    UniformManager<SS_VS, SS_PS>         forwardUniforms;
    InputLayout                          forwardInputLayout;
    ConstantBuffer<Forward_VS_Transform> forwardVSTransform;
    ConstantBuffer<Forward_PS_PerFrame>  forwardPSPerFrame;
    ShaderResourceSlot<SS_PS>           *forwardDiffuseTextureSlot;

    Shader<SS_VS, SS_PS>                shadowShader;
    UniformManager<SS_VS, SS_PS>        shadowUniforms;
    ConstantBuffer<Shadow_VS_Transform> shadowVSTransform;
    InputLayout                         shadowInputLayout;
    RasterizerState                     shadowRasterizerState;

    std::unique_ptr<ForwardShadowMapping> forwardShadowMapping;

public:

    DiffuseSolidBlockEffectCommon();

    void SetForwardRenderParams(const BlockForwardRenderParams &params);

    void SetShadowRenderParams(const BlockShadowRenderParams &params);

    void StartForward(ID3D11ShaderResourceView *diffuseTextureArray) const;

    void EndForward() const;

    void StartShadow() const;

    void EndShadow() const;
};

class DiffuseSolidBlockEffect : public BlockEffect
{
    friend class DiffuseSolidBlockEffectGenerator;

public:

    struct Vertex
    {
        Vec3 position;
        Vec2 texCoord;
        Vec3 normal;
        Vec4 brightness;
        uint32_t texIndex = 0;
    };

    using Builder = NativePartialSectionModelBuilder<DiffuseSolidBlockEffect>;

    const char *GetName() const override;

    bool IsTransparent() const noexcept override;

    std::unique_ptr<ModelBuilder> CreateModelBuilder(const Vec3i &globalSectionPosition) const override;

    void SetForwardRenderParams(const BlockForwardRenderParams &params) const override;

    void SetShadowRenderParams(const BlockShadowRenderParams &params) const override;

    void StartForward() const override;

    void EndForward() const override;

    void StartShadow() const override;

    void EndShadow() const override;

private:

    void Initialize(
        std::shared_ptr<DiffuseSolidBlockEffectCommon> common,
        ShaderResourceView textureArray, std::string name);

    std::shared_ptr<DiffuseSolidBlockEffectCommon> common_;
    ShaderResourceView textureArray_;
    std::string name_;
};

class DiffuseSolidBlockEffectGenerator : public agz::misc::uncopyable_t
{
public:

    DiffuseSolidBlockEffectGenerator(int textureSize, int maxArraySize);

    std::shared_ptr<DiffuseSolidBlockEffect> GetEffectWithTextureSpaces(int textureCount);

    int AddTexture(const Vec4 *textureData);

    void Done();

private:

    void InitializeCurrentEffect();

    std::shared_ptr<DiffuseSolidBlockEffectCommon> common_;

    std::vector<agz::texture::texture2d_t<Vec4>> textureArrayData_;
    int textureSize_;
    int maxArraySize_;

    int nextEffectSemanticsIndex_;
    std::shared_ptr<DiffuseSolidBlockEffect> currentEffect_;
};

VRPG_GAME_END
