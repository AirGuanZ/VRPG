#pragma once

#include <d3d11.h>

#include <VRPG/Base/Common.h>

VRPG_BASE_D3D_BEGIN

ComPtr<ID3D11Buffer> CreateD3D11Buffer(
    const D3D11_BUFFER_DESC &desc, const D3D11_SUBRESOURCE_DATA *initData = nullptr);

ComPtr<ID3D11Texture2D> CreateTexture2D(
    const D3D11_TEXTURE2D_DESC &desc, const D3D11_SUBRESOURCE_DATA *initData = nullptr);

ComPtr<ID3D11ShaderResourceView> CreateShaderResourceView(
    const D3D11_SHADER_RESOURCE_VIEW_DESC &desc, ID3D11Resource *tex);

ComPtr<ID3D11SamplerState> CreateSamplerState(const D3D11_SAMPLER_DESC &desc);

ComPtr<ID3D11RasterizerState> CreateRasterizerState(const D3D11_RASTERIZER_DESC &desc);

VRPG_BASE_D3D_END
