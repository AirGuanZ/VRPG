#include <cassert>

#include <VRPG/Base/D3D/D3DCreate.h>

VRPG_BASE_D3D_BEGIN

ComPtr<ID3D11Buffer> CreateD3D11Buffer(
    const D3D11_BUFFER_DESC &desc, const D3D11_SUBRESOURCE_DATA *initData)
{
    ComPtr<ID3D11Buffer> buffer;
    HRESULT hr = gDevice->CreateBuffer(&desc, initData, buffer.GetAddressOf());
    return SUCCEEDED(hr) ? buffer : nullptr;
}

ComPtr<ID3D11Texture2D> CreateTexture2D(
    const D3D11_TEXTURE2D_DESC &desc, const D3D11_SUBRESOURCE_DATA *initData)
{
    ComPtr<ID3D11Texture2D> texture;
    HRESULT hr = gDevice->CreateTexture2D(&desc, initData, texture.GetAddressOf());
    return SUCCEEDED(hr) ? texture : nullptr;
}

ComPtr<ID3D11ShaderResourceView> CreateShaderResourceView(
    const D3D11_SHADER_RESOURCE_VIEW_DESC &desc, ID3D11Resource *tex)
{
    ComPtr<ID3D11ShaderResourceView> srv;
    HRESULT hr = gDevice->CreateShaderResourceView(tex, &desc, srv.GetAddressOf());
    return SUCCEEDED(hr) ? srv : nullptr;
}

ComPtr<ID3D11SamplerState> CreateSamplerState(const D3D11_SAMPLER_DESC &desc)
{
    ComPtr<ID3D11SamplerState> sampler;
    HRESULT hr = gDevice->CreateSamplerState(&desc, sampler.GetAddressOf());
    return SUCCEEDED(hr) ? sampler : nullptr;
}

ComPtr<ID3D11RasterizerState> CreateRasterizerState(const D3D11_RASTERIZER_DESC &desc)
{
    ComPtr<ID3D11RasterizerState> state;
    HRESULT hr = gDevice->CreateRasterizerState(&desc, state.GetAddressOf());
    return SUCCEEDED(hr) ? state : nullptr;
}

VRPG_BASE_D3D_END
