#include <VRPG/Game/Block/BasicEffect/ShadowMappingUtility.h>
#include <VRPG/Game/Config/GlobalConfig.h>

VRPG_GAME_BEGIN

RasterizerState CreateRasterizerStateForShadowMapping(bool enableBackfaceCulling)
{
    UINT depthBias       = GLOBAL_CONFIG.SHADOW_MAP.depthBias;
    float depthBiasClamp = GLOBAL_CONFIG.SHADOW_MAP.depthBiasClamp;
    float depthSlope     = GLOBAL_CONFIG.SHADOW_MAP.depthSlope;

    RasterizerState state;
    state.Initialize(
        D3D11_FILL_SOLID,
        enableBackfaceCulling ? D3D11_CULL_BACK : D3D11_CULL_NONE,
        false, depthBias, depthBiasClamp, depthSlope);
    return state;
}

ForwardShadowMapping::ForwardShadowMapping(UniformManager<SS_VS, SS_PS> *forwardUniforms)
    : uniforms_(forwardUniforms)
{
    assert(uniforms_);

    vsTransform_.Initialize(true, nullptr);
    uniforms_->GetConstantBufferSlot<SS_VS>("Shadow")->SetBuffer(vsTransform_);

    psShadow_.Initialize(true, nullptr);
    uniforms_->GetConstantBufferSlot<SS_PS>("Shadow")->SetBuffer(psShadow_);

    Sampler shadowSampler;
    const float shadowSamplerBorderColor[] = { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX };
    shadowSampler.Initialize(
        D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
        D3D11_TEXTURE_ADDRESS_BORDER,
        D3D11_TEXTURE_ADDRESS_BORDER,
        D3D11_TEXTURE_ADDRESS_BORDER,
        0, 1, D3D11_COMPARISON_LESS_EQUAL,
        shadowSamplerBorderColor);
    uniforms_->GetSamplerSlot<SS_PS>("ShadowSampler")->SetSampler(shadowSampler);

    nearShadowMapSlot_   = uniforms_->GetShaderResourceSlot<SS_PS>("NearShadowMap");
    middleShadowMapSlot_ = uniforms_->GetShaderResourceSlot<SS_PS>("MiddleShadowMap");
    farShadowMapSlot_    = uniforms_->GetShaderResourceSlot<SS_PS>("FarShadowMap");
    if(!nearShadowMapSlot_ || !middleShadowMapSlot_ || !farShadowMapSlot_)
    {
        throw VRPGGameException("shader resource view slot not found in forward shadow mapping: Near/Middle/Far ShadowMap");
    }
}

void ForwardShadowMapping::SetRenderParams(const BlockForwardRenderParams &params)
{
    vsTransform_.SetValue({
        params.cascadeShadowMaps[0].shadowViewProj,
        params.cascadeShadowMaps[1].shadowViewProj,
        params.cascadeShadowMaps[2].shadowViewProj
    });

    psShadow_.SetValue({
        params.sunlightDirection,
        params.shadowScale,
        params.cascadeShadowMaps[0].PCFStep,
        params.cascadeShadowMaps[1].PCFStep,
        params.cascadeShadowMaps[2].PCFStep,
        0,
        params.cascadeShadowMaps[0].homZLimit,
        params.cascadeShadowMaps[1].homZLimit,
        params.cascadeShadowMaps[2].homZLimit,
        0
    });

    nearShadowMapSRV_   = params.cascadeShadowMaps[0].shadowMapSRV;
    middleShadowMapSRV_ = params.cascadeShadowMaps[1].shadowMapSRV;
    farShadowMapSRV_    = params.cascadeShadowMaps[2].shadowMapSRV;
}

void ForwardShadowMapping::SetRenderParams(
    const Vec3 &sunlightDirection, float shadowScale,
    float nearPCFStep,   float middlePCFStep,   float farPCFStep,
    float nearHomZLimit, float middleHomZLimit, float farHomZLimit,
    ComPtr<ID3D11ShaderResourceView> nearShadowMapSRV,
    ComPtr<ID3D11ShaderResourceView> middleShadowMapSRV,
    ComPtr<ID3D11ShaderResourceView> farShadowMapSRV)
{
    psShadow_.SetValue({
        sunlightDirection,
        shadowScale,
        nearPCFStep,
        middlePCFStep,
        farPCFStep,
        0,
        nearHomZLimit,
        middleHomZLimit,
        farHomZLimit,
        0
        });
    nearShadowMapSRV_   = nearShadowMapSRV;
    middleShadowMapSRV_ = middleShadowMapSRV;
    farShadowMapSRV_    = farShadowMapSRV;
}

void ForwardShadowMapping::Bind()
{
    nearShadowMapSlot_  ->SetShaderResourceView(nearShadowMapSRV_.Get());
    middleShadowMapSlot_->SetShaderResourceView(middleShadowMapSRV_.Get());
    farShadowMapSlot_   ->SetShaderResourceView(farShadowMapSRV_.Get());
}

void ForwardShadowMapping::Unbind()
{
    
}

VRPG_GAME_END
