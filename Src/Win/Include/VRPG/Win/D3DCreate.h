#pragma once

#include <d3d11.h>

#include <VRPG/Win/Common.h>

VRPG_WIN_BEGIN

ID3D11Buffer *CreateD3D11Buffer(
    ID3D11Device *device,
    size_t byteSize,
    const void *initData,
    UINT bindFlag,
    D3D11_USAGE usage,
    UINT cpuAccessFlag,
    UINT structureByteStride,
    UINT miscFlag);

ID3D11Buffer *CreateVertexBuffer(
    size_t byteSize,
    bool dynamic,
    bool streamOutput,
    const void *initData);

ID3D11Buffer *CreateIndexBuffer(
    size_t byteSize,
    bool dynamic,
    const void *initData);

ID3D11Buffer *CreateConstantBuffer(
    size_t byteSize,
    bool dynamic,
    bool cpuWrite,
    const void *initData);

ID3D11Texture2D *CreateTexture2D(
    const D3D11_TEXTURE2D_DESC &desc, const D3D11_SUBRESOURCE_DATA *initData = nullptr);

ID3D11ShaderResourceView *CreateShaderResourceView(
    const D3D11_SHADER_RESOURCE_VIEW_DESC &desc, ID3D11Resource *tex);

VRPG_WIN_END
