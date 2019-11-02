#pragma once

#include <VRPG/World/Block/BasicEffect/NativePartialSectionModel.h>
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

    using Builder = NativePartialSectionModelBuilder<DefaultBlockEffect>;

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
