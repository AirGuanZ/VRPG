#pragma once

#include <VRPG/Game/Block/BasicEffect/NativePartialSectionModel.h>
#include <VRPG/Game/Block/BlockEffect.h>
#include <VRPG/Game/Chunk/ChunkModel.h>
#include <VRPG/Game/Misc/ShadowMappingUtility.h>

VRPG_GAME_BEGIN

class DefaultBlockEffect : public BlockEffect
{
public:

    struct Vertex
    {
        Vec3 position;
        Vec4 brightness;
        Vec3 normal;
    };

    struct Forward_VS_Transform
    {
        Mat4 VP;
    };

    struct Forward_PS_PerFrame
    {
        Vec3 skylight;
        float pad = 0;
    };

    struct Shadow_VS_Transform
    {
        Mat4 VP;
    };

    using Builder = NativePartialSectionModelBuilder<DefaultBlockEffect>;

    DefaultBlockEffect();

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

    void InitializeForward();

    void InitializeShadow();

    Shader<SS_VS, SS_PS>                         forwardShader_;
    UniformManager<SS_VS, SS_PS>                 forwardUniforms_;
    InputLayout                                  forwardInputLayout_;
    mutable ConstantBuffer<Forward_VS_Transform> forwardVSTransform_;
    mutable ConstantBuffer<Forward_PS_PerFrame>  forwardPSPerFrame_;
    std::unique_ptr<ForwardShadowMapping>        forwardShadowMapping_;

    Shader<SS_VS, SS_PS>         shadowShader_;
    UniformManager<SS_VS, SS_PS> shadowUniforms_;
    InputLayout                  shadowInputLayout_;
    RasterizerState              shadowRasterizerState_;
    mutable ConstantBuffer<Shadow_VS_Transform> shadowVSTransform_;
};

VRPG_GAME_END
