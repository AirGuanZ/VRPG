#pragma once

#include <agz/utility/misc.h>

#include <VRPG/Base/D3D/Shader/Shader.h>

VRPG_BASE_D3D_BEGIN

class Effect : public agz::misc::uncopyable_t
{
    ComPtr<ID3D11BlendState>      blendState_;
    ComPtr<ID3D11RasterizerState> rasterizerState_;

    Shader<SS_VS, SS_PS> shader_;

public:

    Effect() = default;
    ~Effect() = default;

    Effect(Effect &&)            noexcept = default;
    Effect &operator=(Effect &&) noexcept = default;

    void InitializeShader(const char *vertexShader, const char *pixelShader)
    {
        agz::misc::scope_guard_t initGuard([&] { shader_.Destroy(); });
        shader_.InitializeStage<SS_VS>(vertexShader);
        shader_.InitializeStage<SS_PS>(pixelShader);
        initGuard.dismiss();
    }

    void DestroyShader()
    {
        shader_.Destroy();
    }

    void SetBlendState(ID3D11BlendState *blendState)
    {
        blendState_ = blendState;
    }

    void SetBlendState(ComPtr<ID3D11BlendState> blendState)
    {
        blendState_ = blendState;
    }

    void SetRasterizerState(ID3D11RasterizerState *rasterizerState)
    {
        rasterizerState_ = rasterizerState;
    }

    void SetRasterizerState(ComPtr<ID3D11RasterizerState> rasterizerState)
    {
        rasterizerState_ = rasterizerState;
    }

    // TODO
};

VRPG_BASE_D3D_END
