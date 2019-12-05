#include <type_traits>

#include <VRPG/Game/Mesh/BasicEffect/DiffuseSolidMeshEffect.h>
#include <VRPG/Game/Config/GlobalConfig.h>
#include <VRPG/Game/Misc/CascadeShadowMapping.h>

VRPG_GAME_BEGIN

namespace DSMEImpl
{
    class EnableShadowImpl
    {
    public:

        EnableShadowImpl();

        void SetShadowRenderParams(const ShadowRenderParams &params) { }

        void StartShadow() const;

        void EndShadow() const;

        void BindShadowVSTransform(const Mat4 &worldViewProj) const;

    private:

        struct VSTransform
        {
            Mat4 worldViewProj;
        };

        Shader<SS_VS, SS_PS>                shader_;
        UniformManager<SS_VS, SS_PS>        uniforms_;
        InputLayout                         inputLayout_;
        ConstantBufferSlot<SS_VS>          *vsTransformSlot_;
        mutable ConstantBuffer<VSTransform> vsTransform_;
        RasterizerState                     rasterizerState_;
    };

    class DisableShadowImpl
    {
    public:

        void SetShadowRenderParams(const ShadowRenderParams &params) { }

        void StartShadow() const { }

        void EndShadow() const { }

        void BindShadowVSTransform(const Mat4 &worldViewProj) const { }
    };

    template<bool EnableShadow>
    class DiffuseSolidMeshEffectImpl : public DiffuseSolidMeshEffect
    {
    public:

        DiffuseSolidMeshEffectImpl();

        void SetForwardRenderParams(const ForwardRenderParams &params) override;

        void StartForward() const override;

        void EndForward() const override;

        void SetShadowRenderParams(const ShadowRenderParams &params) override;

        void StartShadow() const override;

        void EndShadow() const override;

        void BindForwardVSTransform(const Mat4 &world, const Mat4 &worldViewProj) const override;

        void BindForwardPSDiffuseTexture(ID3D11ShaderResourceView *srv) const override;

        void BindForwardPSBrightness(const Vec4 &brightness) const override;

        void BindShadowVSTransform(const Mat4 &worldViewProj) const override;

    private:

        struct ForwardVSTransform
        {
            Mat4 world;
            Mat4 worldViewProj;
        };

        struct ForwardPSPerFrame
        {
            Vec3 skylight;
            float pad = 0;
        };

        struct ForwardPSPerObject
        {
            Vec4 brightness;
        };

        Shader<SS_VS, SS_PS>                       shader_;
        InputLayout                                inputLayout_;

        UniformManager<SS_VS, SS_PS>               uniforms_;
        ConstantBufferSlot<SS_VS>                 *vsTransformSlot_;
        ConstantBufferSlot<SS_PS>                 *psPerFrameSlot_;
        ConstantBufferSlot<SS_PS>                 *psPerObjectSlot_;
        ShaderResourceSlot<SS_PS>                 *diffuseTextureSlot_;

        SamplerSlot<SS_PS>                        *diffuseSamplerSlot_;

        mutable ConstantBuffer<ForwardVSTransform> vsTransform_;
        mutable ConstantBuffer<ForwardPSPerFrame>  psPerFrame_;
        mutable ConstantBuffer<ForwardPSPerObject> psPerObject_;

        std::unique_ptr<ForwardShadowMapping>      shadowMapping_;

        Vec3 skylight_;

        std::conditional_t<EnableShadow, EnableShadowImpl, DisableShadowImpl> shadowImpl_;
    };
}

DSMEImpl::EnableShadowImpl::EnableShadowImpl()
{
    shader_.InitializeStageFromFile<SS_VS>(GLOBAL_CONFIG.ASSET_PATH["Mesh"]["DiffuseSolid"]["ShadowVertexShader"]);
    shader_.InitializeStageFromFile<SS_PS>(GLOBAL_CONFIG.ASSET_PATH["Mesh"]["DiffuseSolid"]["ShadowPixelShader"]);
    if(!shader_.IsAllStagesAvailable())
    {
        throw VRPGGameException("failed to initialize shadow shader for diffuse solid mesh effect");
    }

    uniforms_ = shader_.CreateUniformManager();
    vsTransformSlot_ = uniforms_.GetConstantBufferSlot<SS_VS>("Transform");

    inputLayout_ = InputLayoutBuilder
        ("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, offsetof(DiffuseSolidMeshEffect::Vertex, position))
        .Build(shader_);

    vsTransform_.Initialize(true, nullptr);
    vsTransformSlot_->SetBuffer(vsTransform_);

    rasterizerState_ = CreateRasterizerStateForShadowMapping();
}

void DSMEImpl::EnableShadowImpl::StartShadow() const
{
    shader_         .Bind();
    inputLayout_    .Bind();
    rasterizerState_.Bind();
}

void DSMEImpl::EnableShadowImpl::EndShadow() const
{
    shader_         .Unbind();
    inputLayout_    .Unbind();
    rasterizerState_.Unbind();

    vsTransformSlot_->Unbind();
}

void DSMEImpl::EnableShadowImpl::BindShadowVSTransform(const Mat4 &worldViewProj) const
{
    vsTransform_.SetValue({ worldViewProj });
    vsTransformSlot_->Bind();
}

template<bool EnableShadow>
DSMEImpl::DiffuseSolidMeshEffectImpl<EnableShadow>::DiffuseSolidMeshEffectImpl()
{
    D3D_SHADER_MACRO macros[2] = { GetShadowMappingEnableMacro(), { nullptr, nullptr } };

    shader_.InitializeStageFromFile<SS_VS>(
        GLOBAL_CONFIG.ASSET_PATH["Mesh"]["DiffuseSolid"]["ForwardVertexShader"], macros);
    shader_.InitializeStageFromFile<SS_PS>(
        GLOBAL_CONFIG.ASSET_PATH["Mesh"]["DiffuseSolid"]["ForwardPixelShader"], macros);
    if(!shader_.IsAllStagesAvailable())
    {
        throw VRPGGameException("failed to initialize forward shader for diffuse solid mesh effect");
    }

    inputLayout_ = InputLayoutBuilder
        ("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, offsetof(Vertex, position))
        ("NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, offsetof(Vertex, normal))
        ("TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    offsetof(Vertex, texCoord))
        .Build(shader_);

    uniforms_ = shader_.CreateUniformManager();

    Sampler diffuseSampler;
    diffuseSampler.Initialize(D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT);
    diffuseSamplerSlot_ = uniforms_.GetSamplerSlot<SS_PS>("DiffuseSampler");
    diffuseSamplerSlot_->SetSampler(diffuseSampler);

    vsTransform_.Initialize(true, nullptr);
    psPerFrame_.Initialize(true, nullptr);
    psPerObject_.Initialize(true, nullptr);

    vsTransformSlot_ = uniforms_.GetConstantBufferSlot<SS_VS>("Transform");
    if(!vsTransformSlot_)
    {
        throw VRPGGameException("constant buffer 'Transform' not found in forward vertex shader of diffuse solid mesh effect");
    }
    vsTransformSlot_->SetBuffer(vsTransform_);

    psPerFrameSlot_ = uniforms_.GetConstantBufferSlot<SS_PS>("PerFrame");
    if(!psPerFrameSlot_)
    {
        throw VRPGGameException("constant buffer 'PerFrame' not found in forward pixel shader of diffuse solid mesh effect");
    }
    psPerFrameSlot_->SetBuffer(psPerFrame_);

    psPerObjectSlot_ = uniforms_.GetConstantBufferSlot<SS_PS>("PerObject");
    if(!psPerObjectSlot_)
    {
        throw VRPGGameException("constant buffer 'PerObject' not found in forward pixel shader of diffuse solid mesh effect");
    }
    psPerObjectSlot_->SetBuffer(psPerObject_);

    diffuseTextureSlot_ = uniforms_.GetShaderResourceSlot<SS_PS>("DiffuseTexture");
    if(!diffuseTextureSlot_)
    {
        throw VRPGGameException("shader resource 'DiffuseTexture' not found in forward pixel shader of diffuse solid mesh effect");
    }

    shadowMapping_ = CreateForwardshadowMapping(&uniforms_);
}

template<bool EnableShadow>
void DSMEImpl::DiffuseSolidMeshEffectImpl<EnableShadow>::SetForwardRenderParams(const ForwardRenderParams &params)
{
    shadowMapping_->SetRenderParams(params);
    psPerFrame_.SetValue({ params.skyLight, 0 });
}

template<bool EnableShadow>
void DSMEImpl::DiffuseSolidMeshEffectImpl<EnableShadow>::StartForward() const
{
    shader_             .Bind();
    inputLayout_        .Bind();
    psPerFrameSlot_    ->Bind();
    shadowMapping_     ->StartForward();
    diffuseSamplerSlot_->Bind();
}

template<bool EnableShadow>
void DSMEImpl::DiffuseSolidMeshEffectImpl<EnableShadow>::EndForward() const
{
    vsTransformSlot_   ->Unbind();
    psPerObjectSlot_   ->Unbind();
    diffuseTextureSlot_->Unbind();

    shader_             .Unbind();
    inputLayout_        .Unbind();
    psPerFrameSlot_    ->Unbind();
    shadowMapping_     ->EndForward();
    diffuseSamplerSlot_->Unbind();
}

template<bool EnableShadow>
void DSMEImpl::DiffuseSolidMeshEffectImpl<EnableShadow>::SetShadowRenderParams(const ShadowRenderParams &params)
{
    shadowImpl_.SetShadowRenderParams(params);
}

template<bool EnableShadow>
void DSMEImpl::DiffuseSolidMeshEffectImpl<EnableShadow>::StartShadow() const
{
    shadowImpl_.StartShadow();
}

template<bool EnableShadow>
void DSMEImpl::DiffuseSolidMeshEffectImpl<EnableShadow>::EndShadow() const
{
    shadowImpl_.EndShadow();
}

template<bool EnableShadow>
void DSMEImpl::DiffuseSolidMeshEffectImpl<EnableShadow>::BindForwardVSTransform(const Mat4 &world, const Mat4 &worldViewProj) const
{
    vsTransform_.SetValue({ world, worldViewProj });
    vsTransformSlot_->Bind();
}

template<bool EnableShadow>
void DSMEImpl::DiffuseSolidMeshEffectImpl<EnableShadow>::BindForwardPSBrightness(const Vec4 &brightness) const
{
    psPerObject_.SetValue({ brightness });
    psPerObjectSlot_->Bind();
}

template<bool EnableShadow>
void DSMEImpl::DiffuseSolidMeshEffectImpl<EnableShadow>::BindForwardPSDiffuseTexture(ID3D11ShaderResourceView *srv) const
{
    diffuseTextureSlot_->SetShaderResourceView(srv);
    diffuseTextureSlot_->Bind();
}

template<bool EnableShadow>
void DSMEImpl::DiffuseSolidMeshEffectImpl<EnableShadow>::BindShadowVSTransform(const Mat4 &worldViewProj) const
{
    shadowImpl_.BindShadowVSTransform(worldViewProj);
}

std::shared_ptr<DiffuseSolidMeshEffect> CreateDiffuseSolidMeshEffect()
{
    if(GLOBAL_CONFIG.SHADOW_MAP.enable)
    {
        return std::make_shared<DSMEImpl::DiffuseSolidMeshEffectImpl<true>>();
    }
    return std::make_shared<DSMEImpl::DiffuseSolidMeshEffectImpl<false>>();
}

VRPG_GAME_END
