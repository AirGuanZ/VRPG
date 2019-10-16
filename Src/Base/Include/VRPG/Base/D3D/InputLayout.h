#pragma once

#include <vector>

#include <d3d11.h>

#include <VRPG/Base/D3D/Shader.h>

VRPG_BASE_D3D_BEGIN

class InputLayoutBuilder
{
    std::vector<D3D11_INPUT_ELEMENT_DESC> descs_;
    UINT inputSlot_ = 0;

public:

    InputLayoutBuilder() = default;

    InputLayoutBuilder(
        const char *semantic,
        UINT        semanticIndex,
        DXGI_FORMAT format,
        UINT        byteOffset)
        : InputLayoutBuilder()
    {
        Append(semantic, semanticIndex, format, byteOffset);
    }

    InputLayoutBuilder(
        const char *semantic,
        UINT        semanticIndex,
        DXGI_FORMAT format,
        UINT        byteOffset,
        UINT        instanceDataStepRate)
        : InputLayoutBuilder()
    {
        Append(semantic, semanticIndex, format, byteOffset, instanceDataStepRate);
    }

    InputLayoutBuilder &Append(
        const char *semantic,
        UINT        semanticIndex,
        DXGI_FORMAT format,
        UINT        byteOffset)
    {
        descs_.push_back({
            semantic,
            semanticIndex,
            format,
            inputSlot_,
            byteOffset,
            D3D11_INPUT_PER_VERTEX_DATA,
            0
        });
        return *this;
    }

    InputLayoutBuilder &Append(
        const char *semantic,
        UINT        semanticIndex,
        DXGI_FORMAT format,
        UINT        byteOffset,
        UINT        instanceDataStepRate)
    {
        descs_.push_back({
            semantic,
            semanticIndex,
            format,
            inputSlot_,
            byteOffset,
            D3D11_INPUT_PER_INSTANCE_DATA,
            instanceDataStepRate
        });
        return *this;
    }

    InputLayoutBuilder &operator()(
        const char *semantic,
        UINT        semanticIndex,
        DXGI_FORMAT format,
        UINT        byteOffset)
    {
        return Append(semantic, semanticIndex, format, byteOffset);
    }

    InputLayoutBuilder &operator()(
        const char *semantic,
        UINT        semanticIndex,
        DXGI_FORMAT format,
        UINT        byteOffset,
        UINT        instanceDataStepRate)
    {
        return Append(semantic, semanticIndex, format, byteOffset, instanceDataStepRate);
    }

    InputLayoutBuilder &NextInputSlot() noexcept
    {
        ++inputSlot_;
        return *this;
    }

    InputLayoutBuilder &SetInputSlot(UINT inputSlot) noexcept
    {
        inputSlot_ = inputSlot;
        return *this;
    }

    template<ShaderStage...STAGES>
    ComPtr<ID3D11InputLayout> Build(const Shader<STAGES...> &shader) const
    {
        return Build(shader.GetVertexShaderByteCode());
    }

    ComPtr<ID3D11InputLayout> Build(ID3D10Blob *shaderByteCode) const
    {
        ComPtr<ID3D11InputLayout> inputLayout;
        HRESULT hr = gDevice->CreateInputLayout(
            descs_.data(), UINT(descs_.size()),
            shaderByteCode->GetBufferPointer(),
            shaderByteCode->GetBufferSize(),
            inputLayout.GetAddressOf());
        return SUCCEEDED(hr) ? inputLayout : nullptr;
    }
};

VRPG_BASE_D3D_END
