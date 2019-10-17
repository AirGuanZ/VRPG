#pragma once

#include <VRPG/Base/D3D/D3DCreate.h>

VRPG_BASE_D3D_BEGIN

class BlendState
{
    ComPtr<ID3D11BlendState> blendState_;
    float blendFactor_[4] = { 1, 1, 1, 1 };
    UINT sampleMask_ = 0xffffffff;

public:

    static const float *_allOnes() noexcept
    {
        static const float ret[] = { 1, 1, 1, 1 };
        return ret;
    }

    BlendState() = default;

    explicit BlendState(ComPtr<ID3D11BlendState> blendState, const float blendFactor[] = _allOnes(), UINT sampleMask = 0xffffffff) noexcept
        : blendState_(std::move(blendState)),
          blendFactor_{ blendFactor[0], blendFactor[1], blendFactor[2], blendFactor[3] },
          sampleMask_(sampleMask)
    {
        
    }

    bool IsAvailable() const noexcept
    {
        return blendState_ != nullptr;
    }

    void Destroy()
    {
        blendState_.Reset();
        blendFactor_[0] = blendFactor_[1] = blendFactor_[2] = blendFactor_[3] = 1;
        sampleMask_ = 0xffffffff;
    }

    const float *GetBlendFactor() const noexcept
    {
        return blendFactor_;
    }

    UINT GetSampleMask() const noexcept
    {
        return sampleMask_;
    }

    ID3D11BlendState *GetBlendState() const noexcept
    {
        return blendState_.Get();
    }

    operator ID3D11BlendState*() const noexcept
    {
        return blendState_.Get();
    }

    void Bind() const
    {
        gDeviceContext->OMSetBlendState(blendState_.Get(), blendFactor_, sampleMask_);
    }

    void Unbind() const
    {
        gDeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
    }
};

VRPG_BASE_D3D_END
