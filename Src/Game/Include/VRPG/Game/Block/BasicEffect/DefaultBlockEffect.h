#pragma once

#include <VRPG/Game/Block/BasicEffect/NativePartialSectionModel.h>
#include <VRPG/Game/Block/BlockEffect.h>
#include <VRPG/Game/Chunk/ChunkModel.h>

VRPG_GAME_BEGIN

class DefaultBlockEffect : public BlockEffect
{
public:

    struct Vertex
    {
        Vec3 position;
        Vec4 brightness;
    };

    struct Forward_VS_Transform
    {
        Mat4 WVP;
    };

    struct Forward_PS_Sky
    {
        Vec3 skylight;
        float pad = 0;
    };

    using Builder = NativePartialSectionModelBuilder<DefaultBlockEffect>;

    DefaultBlockEffect();

    const char *GetName() const override;

    bool IsTransparent() const noexcept override;

    void StartForward() const override;

    void EndForward() const override;

    std::unique_ptr<PartialSectionModelBuilder> CreateModelBuilder(const Vec3i &globalSectionPosition) const override;

    void SetForwardRenderParams(const BlockForwardRenderParams &params) const override;

private:

    Shader<SS_VS, SS_PS>                         forwardShader_;
    UniformManager<SS_VS, SS_PS>                 forwardUniforms_;
    InputLayout                                  forwardInputLayout_;
    mutable ConstantBuffer<Forward_VS_Transform> forwardVSTransform_;
    mutable ConstantBuffer<Forward_PS_Sky>       forwardPSSky_;
};

VRPG_GAME_END
