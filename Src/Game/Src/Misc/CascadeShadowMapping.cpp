#include <VRPG/Game/Config/GlobalConfig.h>
#include <VRPG/Game/Misc/CascadeShadowMapping.h>
#include <VRPG/Game/World/Chunk/ChunkRenderer.h>

VRPG_GAME_BEGIN

class DisableCascadeShadowMapping : public CascadeShadowMapping
{
public:

    void UpdateCSMParams(const Camera &camera) override { }

    void StartNear() override { }

    void EndNear() override { }

    void StartMiddle() override { }

    void EndMiddle() override { }

    void StartFar() override { }

    void EndFar() override { }

    void FillNearShadowParams(ShadowRenderParams &params) const noexcept override { params.shadowViewProj = Mat4::identity(); }

    void FillMiddleShadowParams(ShadowRenderParams &params) const noexcept override { params.shadowViewProj = Mat4::identity(); }

    void FillFarShadowParams(ShadowRenderParams &params) const noexcept override { params.shadowViewProj = Mat4::identity(); }

    void FillForwardParams(ForwardRenderParams &params) override { }
};

class EnableCascadeShadowMapping : public CascadeShadowMapping
{
public:

    EnableCascadeShadowMapping();

    void UpdateCSMParams(const Camera &camera) override;

    void StartNear() override;

    void EndNear() override;

    void StartMiddle() override;

    void EndMiddle() override;

    void StartFar() override;

    void EndFar() override;

    void FillNearShadowParams(ShadowRenderParams &params) const noexcept override;

    void FillMiddleShadowParams(ShadowRenderParams &params) const noexcept override;

    void FillFarShadowParams(ShadowRenderParams &params) const noexcept override;

    void FillForwardParams(ForwardRenderParams &params) override;

private:

    static void UpdateViewProj(const Camera &camera, Mat4 viewProj[3], float cascadeZLimit[3]);

    Mat4 viewProj_[3];
    float cascadeZLimit_[3];

    std::unique_ptr<Base::ShadowMap> nearSM_;
    std::unique_ptr<Base::ShadowMap> middleSM_;
    std::unique_ptr<Base::ShadowMap> farSM_;
};

class DisabledForwardShadowMapping : public ForwardShadowMapping
{
public:

    void SetRenderParams(const ForwardRenderParams &params) override { }

    void StartForward() override { }

    void EndForward() override { }
};

class EnabledForwardShadowMapping : public ForwardShadowMapping
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

        float enableShadow  = 1;
        float nearPCFStep   = 1.0f / 4096;
        float moddlePCFStep = 1.0f / 4096;
        float farPCFStep    = 1.0f / 4096;
        
        float nearHomZLimit   = 1;
        float middleHomZLimit = 2;
        float farHomZLimit    = 3;
        float pad1            = 0;
    };

    UniformManager<SS_VS, SS_PS>    *uniforms_;
    ConstantBuffer<VSTransform>      vsTransform_;
    ConstantBuffer<PSShadow>         psShadow_;

    SamplerSlot<SS_PS>              *shadowSamplerSlot_;

    ConstantBufferSlot<SS_VS>       *vsTransformSlot_;
    ConstantBufferSlot<SS_PS>       *psShadowSlot_;

    ShaderResourceSlot<SS_PS>       *nearShadowMapSlot_;
    ComPtr<ID3D11ShaderResourceView> nearShadowMapSRV_;

    ShaderResourceSlot<SS_PS>       *middleShadowMapSlot_;
    ComPtr<ID3D11ShaderResourceView> middleShadowMapSRV_;

    ShaderResourceSlot<SS_PS>       *farShadowMapSlot_;
    ComPtr<ID3D11ShaderResourceView> farShadowMapSRV_;

public:

    explicit EnabledForwardShadowMapping(UniformManager<SS_VS, SS_PS> *forwardUniforms);

    void SetRenderParams(const ForwardRenderParams &params) override;

    void StartForward() override;

    void EndForward() override;
};

std::unique_ptr<CascadeShadowMapping> CreateCascadeShadowMapping()
{
    if(GLOBAL_CONFIG.SHADOW_MAP.enable)
    {
        return std::make_unique<EnableCascadeShadowMapping>();
    }
    return std::make_unique<DisableCascadeShadowMapping>();
}

EnableCascadeShadowMapping::EnableCascadeShadowMapping()
    : cascadeZLimit_{ 1, 1, 1 }
{
    nearSM_ = std::make_unique<Base::ShadowMap>(
        GLOBAL_CONFIG.SHADOW_MAP.resolution[0], GLOBAL_CONFIG.SHADOW_MAP.resolution[0]);
    middleSM_ = std::make_unique<Base::ShadowMap>(
        GLOBAL_CONFIG.SHADOW_MAP.resolution[1], GLOBAL_CONFIG.SHADOW_MAP.resolution[1]);
    farSM_ = std::make_unique<Base::ShadowMap>(
        GLOBAL_CONFIG.SHADOW_MAP.resolution[2], GLOBAL_CONFIG.SHADOW_MAP.resolution[2]);
}

void EnableCascadeShadowMapping::UpdateCSMParams(const Camera &camera)
{
    UpdateViewProj(camera, viewProj_, cascadeZLimit_);
}

void EnableCascadeShadowMapping::StartNear()
{
    nearSM_->Begin();
}

void EnableCascadeShadowMapping::EndNear()
{
    nearSM_->End();
}

void EnableCascadeShadowMapping::StartMiddle()
{
    middleSM_->Begin();
}

void EnableCascadeShadowMapping::EndMiddle()
{
    middleSM_->End();
}

void EnableCascadeShadowMapping::StartFar()
{
    farSM_->Begin();
}

void EnableCascadeShadowMapping::EndFar()
{
    farSM_->End();
}

void EnableCascadeShadowMapping::FillNearShadowParams(ShadowRenderParams &params) const noexcept
{
    params.shadowViewProj = viewProj_[0];
}

void EnableCascadeShadowMapping::FillMiddleShadowParams(ShadowRenderParams &params) const noexcept
{
    params.shadowViewProj = viewProj_[1];
}

void EnableCascadeShadowMapping::FillFarShadowParams(ShadowRenderParams &params) const noexcept
{
    params.shadowViewProj = viewProj_[2];
}

namespace
{
    const Vec3 SUN_DIR = Vec3(5, 6, 7).normalize();
}

void EnableCascadeShadowMapping::FillForwardParams(ForwardRenderParams &params)
{
    params.shadowScale       = 0.2f;
    params.sunlightDirection = SUN_DIR;

    params.cascadeShadowMaps[0].shadowMapSRV = nearSM_->GetSRV();
    params.cascadeShadowMaps[1].shadowMapSRV = middleSM_->GetSRV();
    params.cascadeShadowMaps[2].shadowMapSRV = farSM_->GetSRV();

    params.cascadeShadowMaps[0].PCFStep = 1 / static_cast<float>(GLOBAL_CONFIG.SHADOW_MAP.resolution[0]);
    params.cascadeShadowMaps[1].PCFStep = 1 / static_cast<float>(GLOBAL_CONFIG.SHADOW_MAP.resolution[1]);
    params.cascadeShadowMaps[2].PCFStep = 1 / static_cast<float>(GLOBAL_CONFIG.SHADOW_MAP.resolution[2]);

    params.cascadeShadowMaps[0].shadowViewProj = viewProj_[0];
    params.cascadeShadowMaps[1].shadowViewProj = viewProj_[1];
    params.cascadeShadowMaps[2].shadowViewProj = viewProj_[2];

    params.cascadeShadowMaps[0].homZLimit = cascadeZLimit_[0];
    params.cascadeShadowMaps[1].homZLimit = cascadeZLimit_[1];
    params.cascadeShadowMaps[2].homZLimit = cascadeZLimit_[2];
}

void EnableCascadeShadowMapping::UpdateViewProj(const Camera &camera, Mat4 viewProj[3], float cascadeZLimit[3])
{
    float FOVy   = camera.GetFOVy();
    float wOverH = camera.GetWOverH();

    Vec3 shadowDst = camera.GetPosition();
    shadowDst.x = std::floor(shadowDst.x / 50) * 50;
    shadowDst.y = std::floor(shadowDst.y / 50) * 50;
    shadowDst.z = std::floor(shadowDst.z / 50) * 50;

    Mat4 shadowView = Trans4::look_at(
        shadowDst + 600.0f * SUN_DIR, shadowDst, Vec3(0, 1, 0));
    Mat4 viewToShadow = camera.GetViewMatrix().inv() * shadowView;

    auto constructSingleShadowMapVP = [&](float nearD, float farD, int shadowMapResolution)
    {
        float nearYOri = nearD * std::tan(FOVy / 2);
        float nearXOri = wOverH * nearYOri;
        Vec3 nearPoints[4] =
        {
            { +nearXOri, +nearYOri, nearD },
            { +nearXOri, -nearYOri, nearD },
            { -nearXOri, +nearYOri, nearD },
            { -nearXOri, -nearYOri, nearD }
        };

        float farYOri = farD * std::tan(FOVy / 2);
        float farXOri = wOverH * farYOri;
        Vec3 farPoints[4] =
        {
            { +farXOri, +farYOri, farD },
            { +farXOri, -farYOri, farD },
            { -farXOri, +farYOri, farD },
            { -farXOri, -farYOri, farD }
        };

        Vec3 boundSphereCenter;
        for(int i = 0; i < 4; ++i)
        {
            nearPoints[i] = (Vec4(nearPoints[i], 1) * viewToShadow).xyz();
            farPoints [i] = (Vec4(farPoints [i], 1) * viewToShadow).xyz();
            boundSphereCenter += nearPoints[i] + farPoints[i];
        }
        boundSphereCenter /= 8.0f;

        float boundSphereRadius = 0;
        for(int i = 0; i < 4; ++i)
        {
            float nearDistance = (nearPoints[i] - boundSphereCenter).length_square();
            float farDistance  = (farPoints[i] - boundSphereCenter) .length_square();
            boundSphereRadius = (std::max)(
                boundSphereRadius, (std::max)(nearDistance, farDistance));
        }
        boundSphereRadius = 1.01f * std::sqrt(boundSphereRadius);
        
        float minX = boundSphereCenter.x - boundSphereRadius;
        float minY = boundSphereCenter.y - boundSphereRadius;
        float maxX = boundSphereCenter.x + boundSphereRadius;
        float maxY = boundSphereCenter.y + boundSphereRadius;
        float maxZ = boundSphereCenter.z + boundSphereRadius;

        //float minX = (std::numeric_limits<float>::max)();
        //float minY = (std::numeric_limits<float>::max)();
        //float maxX = (std::numeric_limits<float>::lowest)();
        //float maxY = (std::numeric_limits<float>::lowest)();
        //float maxZ = (std::numeric_limits<float>::lowest)();
        //
        //for(int i = 0; i < 4; ++i)
        //{
        //    minX = (std::min)(minX, nearPoints[i].x);
        //    minY = (std::min)(minY, nearPoints[i].y);
        //    maxX = (std::max)(maxX, nearPoints[i].x);
        //    maxY = (std::max)(maxY, nearPoints[i].y);
        //    maxZ = (std::max)(maxZ, nearPoints[i].z);
        //
        //    minX = (std::min)(minX, farPoints[i].x);
        //    minY = (std::min)(minY, farPoints[i].y);
        //    maxX = (std::max)(maxX, farPoints[i].x);
        //    maxY = (std::max)(maxY, farPoints[i].y);
        //    maxZ = (std::max)(maxZ, farPoints[i].z);
        //}

        float unitsPerPixelX = (maxX - minX) / shadowMapResolution;
        float unitsPerPixelY = (maxY - minY) / shadowMapResolution;
        minX = std::floor(minX / unitsPerPixelX) * unitsPerPixelX;
        maxX = std::floor(maxX / unitsPerPixelX) * unitsPerPixelX;
        minY = std::floor(minY / unitsPerPixelY) * unitsPerPixelY;
        maxY = std::floor(maxY / unitsPerPixelY) * unitsPerPixelY;

        Mat4 shadowProj = Trans4::orthographic(
            minX, maxX, maxY, minY, 300, maxZ + 5);

        return shadowView * shadowProj;
    };

    float distance0 = camera.GetNearDistance();
    float distance1 = GLOBAL_CONFIG.SHADOW_MAP.distance;
    float distance2 = distance1 + 2.0f * (distance1 - distance0);
    float distance3 = distance2 + 2.0f * (distance2 - distance1);

    auto computeHomZLimit = [proj = camera.GetProjMatrix()](float maxZ)
    {
        Vec4 clipV = Vec4(0, 0, maxZ, 1) * proj;
        return clipV.z;
    };

    viewProj[0] = constructSingleShadowMapVP(distance0, distance1, GLOBAL_CONFIG.SHADOW_MAP.resolution[0]);
    viewProj[1] = constructSingleShadowMapVP(distance1, distance2, GLOBAL_CONFIG.SHADOW_MAP.resolution[1]);
    viewProj[2] = constructSingleShadowMapVP(distance2, distance3, GLOBAL_CONFIG.SHADOW_MAP.resolution[2]);

    cascadeZLimit[0] = computeHomZLimit(distance1);
    cascadeZLimit[1] = computeHomZLimit(distance2);
    cascadeZLimit[2] = computeHomZLimit(distance3);
}

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

std::unique_ptr<ForwardShadowMapping> CreateForwardshadowMapping(
    UniformManager<SS_VS, SS_PS> *forwardUniforms)
{
    if(GLOBAL_CONFIG.SHADOW_MAP.enable)
    {
        return std::make_unique<EnabledForwardShadowMapping>(forwardUniforms);
    }
    return std::make_unique<DisabledForwardShadowMapping>();
}

D3D_SHADER_MACRO GetShadowMappingEnableMacro()
{
    D3D_SHADER_MACRO macro;
    macro.Name       = "ENABLE_SHADOW";
    macro.Definition = GLOBAL_CONFIG.SHADOW_MAP.enable ? "1" : "0";
    return macro;
}

EnabledForwardShadowMapping::EnabledForwardShadowMapping(UniformManager<SS_VS, SS_PS> *forwardUniforms)
    : uniforms_(forwardUniforms)
{
    assert(uniforms_);

    vsTransformSlot_ = uniforms_->GetConstantBufferSlot<SS_VS>("Shadow");
    psShadowSlot_    = uniforms_->GetConstantBufferSlot<SS_PS>("Shadow");

    vsTransform_.Initialize(true, nullptr);
    vsTransformSlot_->SetBuffer(vsTransform_);

    psShadow_.Initialize(true, nullptr);
    psShadowSlot_->SetBuffer(psShadow_);

    shadowSamplerSlot_ = uniforms_->GetSamplerSlot<SS_PS>("ShadowSampler");

    Sampler shadowSampler;
    const float shadowSamplerBorderColor[] = { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX };
    shadowSampler.Initialize(
        D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
        D3D11_TEXTURE_ADDRESS_BORDER,
        D3D11_TEXTURE_ADDRESS_BORDER,
        D3D11_TEXTURE_ADDRESS_BORDER,
        0, 1, D3D11_COMPARISON_LESS_EQUAL,
        shadowSamplerBorderColor);
    /*shadowSampler.Initialize(
            D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT,
            D3D11_TEXTURE_ADDRESS_BORDER,
            D3D11_TEXTURE_ADDRESS_BORDER,
            D3D11_TEXTURE_ADDRESS_BORDER,
            0, 1, D3D11_COMPARISON_LESS_EQUAL,
            shadowSamplerBorderColor);*/
    shadowSamplerSlot_->SetSampler(shadowSampler);

    nearShadowMapSlot_   = uniforms_->GetShaderResourceSlot<SS_PS>("NearShadowMap");
    middleShadowMapSlot_ = uniforms_->GetShaderResourceSlot<SS_PS>("MiddleShadowMap");
    farShadowMapSlot_    = uniforms_->GetShaderResourceSlot<SS_PS>("FarShadowMap");
    if(!nearShadowMapSlot_ || !middleShadowMapSlot_ || !farShadowMapSlot_)
    {
        throw VRPGGameException("shader resource view slot not found in forward shadow mapping: Near/Middle/Far ShadowMap");
    }
}

void EnabledForwardShadowMapping::SetRenderParams(const ForwardRenderParams &params)
{
    vsTransform_.SetValue({
        params.cascadeShadowMaps[0].shadowViewProj,
        params.cascadeShadowMaps[1].shadowViewProj,
        params.cascadeShadowMaps[2].shadowViewProj
    });

    psShadow_.SetValue({
        params.sunlightDirection,
        params.shadowScale,
        GLOBAL_CONFIG.SHADOW_MAP.enable ? 1.0f : 0.0f,
        params.cascadeShadowMaps[0].PCFStep,
        params.cascadeShadowMaps[1].PCFStep,
        params.cascadeShadowMaps[2].PCFStep,
        params.cascadeShadowMaps[0].homZLimit,
        params.cascadeShadowMaps[1].homZLimit,
        params.cascadeShadowMaps[2].homZLimit,
        0
    });

    nearShadowMapSRV_   = params.cascadeShadowMaps[0].shadowMapSRV;
    middleShadowMapSRV_ = params.cascadeShadowMaps[1].shadowMapSRV;
    farShadowMapSRV_    = params.cascadeShadowMaps[2].shadowMapSRV;
}

void EnabledForwardShadowMapping::StartForward()
{
    nearShadowMapSlot_  ->SetShaderResourceView(nearShadowMapSRV_.Get());
    middleShadowMapSlot_->SetShaderResourceView(middleShadowMapSRV_.Get());
    farShadowMapSlot_   ->SetShaderResourceView(farShadowMapSRV_.Get());

    nearShadowMapSlot_  ->Bind();
    middleShadowMapSlot_->Bind();
    farShadowMapSlot_   ->Bind();

    shadowSamplerSlot_->Bind();

    vsTransformSlot_->Bind();
    psShadowSlot_   ->Bind();
}

void EnabledForwardShadowMapping::EndForward()
{
    nearShadowMapSlot_  ->Unbind();
    middleShadowMapSlot_->Unbind();
    farShadowMapSlot_   ->Unbind();

    shadowSamplerSlot_->Unbind();

    vsTransformSlot_->Unbind();
    psShadowSlot_   ->Unbind();
}

VRPG_GAME_END
