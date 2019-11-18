#pragma once

#include <VRPG/Game/Block/BlockEffect.h>

VRPG_GAME_BEGIN

RasterizerState CreateRasterizerStateForShadowMapping(bool enableBackfaceCulling = true);

class ForwardShadowMapping
{
    struct VSTransform
    {
        Mat4 nearShadowVP;
        Mat4 middleShadowVP;
        Mat4 farShadowVP;
    };

    struct PSShadow
    {
        Vec3 sunlightDirection;
        float shadowScale   = 1;

        float nearPCFStep   = 1.0f / 4096;
        float moddlePCFStep = 1.0f / 4096;
        float farPCFStep    = 1.0f / 4096;
        float pad0          = 0;

        float nearHomZLimit   = 1;
        float middleHomZLimit = 2;
        float farHomZLimit    = 3;
        float pad1            = 0;
    };

    UniformManager<SS_VS, SS_PS>    *uniforms_;
    ConstantBuffer<VSTransform>      vsTransform_;
    ConstantBuffer<PSShadow>         psShadow_;

    ShaderResourceSlot<SS_PS>       *nearShadowMapSlot_;
    ComPtr<ID3D11ShaderResourceView> nearShadowMapSRV_;

    ShaderResourceSlot<SS_PS>       *middleShadowMapSlot_;
    ComPtr<ID3D11ShaderResourceView> middleShadowMapSRV_;

    ShaderResourceSlot<SS_PS>       *farShadowMapSlot_;
    ComPtr<ID3D11ShaderResourceView> farShadowMapSRV_;

public:

    explicit ForwardShadowMapping(UniformManager<SS_VS, SS_PS> *forwardUniforms);

    void SetRenderParams(const BlockForwardRenderParams &params);

    void Bind();

    void Unbind();
};

VRPG_GAME_END
