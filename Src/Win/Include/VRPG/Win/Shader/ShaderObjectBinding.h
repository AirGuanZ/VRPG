#pragma once

#include <d3d11.h>

#include <VRPG/Win/Common.h>

VRPG_WIN_BEGIN

enum class ShaderStage
{
    VS = 0,
    HS = 1,
    DS = 2,
    GS = 3,
    PS = 4
};

template<ShaderStage STAGE>
void BindConstantBuffer(UINT slot, ID3D11Buffer *buffer);

template<>
inline void BindConstantBuffer<ShaderStage::VS>(UINT slot, ID3D11Buffer *buffer)
{
    gDeviceContext->VSSetConstantBuffers(slot, 1, &buffer);
}

template<>
inline void BindConstantBuffer<ShaderStage::HS>(UINT slot, ID3D11Buffer *buffer)
{
    gDeviceContext->HSSetConstantBuffers(slot, 1, &buffer);
}

template<>
inline void BindConstantBuffer<ShaderStage::DS>(UINT slot, ID3D11Buffer *buffer)
{
    gDeviceContext->DSSetConstantBuffers(slot, 1, &buffer);
}

template<>
inline void BindConstantBuffer<ShaderStage::GS>(UINT slot, ID3D11Buffer *buffer)
{
    gDeviceContext->GSSetConstantBuffers(slot, 1, &buffer);
}

template<>
inline void BindConstantBuffer<ShaderStage::PS>(UINT slot, ID3D11Buffer *buffer)
{
    gDeviceContext->PSSetConstantBuffers(slot, 1, &buffer);
}

VRPG_WIN_END
