#pragma once

#include <VRPG/Base/D3D/Shader.h>

VRPG_BASE_D3D_BEGIN

class PipelineState : public agz::misc::uncopyable_t
{
    ComPtr<ID3D11RasterizerState> rasterizerState_;
    ComPtr<ID3D11InputLayout>     inputLayout_;

    ComPtr<ID3D11BlendState> blendState_;
    FLOAT blendFactor_[4] = { 1, 1, 1, 1 };
    UINT blendSampleMask_ = 0xFFFFFFFF;

    D3D11_PRIMITIVE_TOPOLOGY primitiveTopology_ = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    Shader<SS_VS, SS_PS> shader_;

public:

    PipelineState() = default;
    ~PipelineState() = default;

    PipelineState(PipelineState &&)            noexcept = default;
    PipelineState &operator=(PipelineState &&) noexcept = default;

    void Destroy()
    {
        rasterizerState_.Reset();
        blendState_.Reset();
        inputLayout_.Reset();

        shader_.Destroy();
    }

    void SetShader(Shader<SS_VS, SS_PS> &&shader)
    {
        shader_ = std::move(shader);
    }

    void InitializeShader(const char *vertexShader, const char *pixelShader)
    {
        agz::misc::scope_guard_t shaderGuard([&] { shader_.Destroy(); });
        shader_.InitializeStage<SS_VS>(vertexShader);
        shader_.InitializeStage<SS_PS>(pixelShader);
        shaderGuard.dismiss();
    }

    void DestroyShader()
    {
        shader_.Destroy();
    }

    void SetRasterizerState(ID3D11RasterizerState *state) noexcept
    {
        rasterizerState_ = state;
    }

    void SetInputLayout(ID3D11InputLayout *inputLayout) noexcept
    {
        inputLayout_ = inputLayout;
    }

    void SetBlendState(ID3D11BlendState *state) noexcept
    {
        blendState_ = state;
    }

    void SetBlendFactor(const FLOAT blendFactor[4]) noexcept
    {
        blendFactor_[0] = blendFactor[0];
        blendFactor_[1] = blendFactor[1];
        blendFactor_[2] = blendFactor[2];
        blendFactor_[3] = blendFactor[3];
    }

    void SetBlendSampleMask(UINT sampleMask) noexcept
    {
        blendSampleMask_ = sampleMask;
    }

    void SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topology) noexcept
    {
        primitiveTopology_ = topology;
    }

    const Shader<SS_VS, SS_PS> &GetShader() const noexcept
    {
        return shader_;
    }

    void Bind() const
    {
        gDeviceContext->RSSetState(rasterizerState_.Get());
        gDeviceContext->OMSetBlendState(blendState_.Get(), blendFactor_, blendSampleMask_);
        gDeviceContext->IASetInputLayout(inputLayout_.Get());
        gDeviceContext->IASetPrimitiveTopology(primitiveTopology_);
        shader_.Bind();
    }

    void Unbind() const
    {
        gDeviceContext->RSSetState(nullptr);
        gDeviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
        gDeviceContext->IASetInputLayout(nullptr);
        gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        shader_.Unbind();
    }
};

VRPG_BASE_D3D_END
