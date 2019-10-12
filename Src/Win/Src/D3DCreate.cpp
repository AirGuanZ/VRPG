#include <cassert>

#include <VRPG/Win/D3DCreate.h>

VRPG_WIN_BEGIN

ID3D11Buffer *CreateD3D11Buffer(
    ID3D11Device *device,
    size_t byteSize,
    const void *initData,
    UINT bindFlag,
    D3D11_USAGE usage,
    UINT cpuAccessFlag,
    UINT structureByteStride,
    UINT miscFlag)
{
    assert(device);

    D3D11_BUFFER_DESC desc;
    desc.ByteWidth = UINT(byteSize);
    desc.Usage = usage;
    desc.BindFlags = bindFlag;
    desc.CPUAccessFlags = cpuAccessFlag;
    desc.MiscFlags = miscFlag;
    desc.StructureByteStride = structureByteStride;

    D3D11_SUBRESOURCE_DATA subrscData;
    ZeroMemory(&subrscData, sizeof(subrscData));
    subrscData.pSysMem = initData;

    ID3D11Buffer *buffer;
    HRESULT hr = device->CreateBuffer(&desc, initData ? &subrscData : nullptr, &buffer);
    return FAILED(hr) ? nullptr : buffer;
}

ID3D11Buffer *CreateVertexBuffer(
    size_t byteSize,
    bool dynamic,
    bool streamOutput,
    const void *initData)
{
    UINT bindFlag = D3D11_BIND_VERTEX_BUFFER;
    if(streamOutput)
        bindFlag |= D3D11_BIND_STREAM_OUTPUT;

    D3D11_USAGE usage;
    UINT cpuAccessFlag;
    if(dynamic)
    {
        usage = D3D11_USAGE_DYNAMIC;
        cpuAccessFlag = D3D11_CPU_ACCESS_WRITE;
    }
    else
    {
        usage = D3D11_USAGE_IMMUTABLE;
        cpuAccessFlag = 0;
    }

    return CreateD3D11Buffer(gDevice, byteSize, initData, bindFlag, usage, cpuAccessFlag, 0, 0);
}


ID3D11Buffer *CreateIndexBuffer(
    size_t byteSize,
    bool dynamic,
    const void *initData)
{
    UINT bindFlag = D3D11_BIND_INDEX_BUFFER;

    D3D11_USAGE usage;
    UINT cpuAccessFlag;
    if(dynamic)
    {
        usage = D3D11_USAGE_DYNAMIC;
        cpuAccessFlag = D3D11_CPU_ACCESS_WRITE;
    }
    else
    {
        usage = D3D11_USAGE_IMMUTABLE;
        cpuAccessFlag = 0;
    }

    return CreateD3D11Buffer(gDevice, byteSize, initData, bindFlag, usage, cpuAccessFlag, 0, 0);
}

ID3D11Buffer *CreateConstantBuffer(
    size_t byteSize,
    bool dynamic,
    bool cpuWrite,
    const void *initData)
{
    D3D11_USAGE usage;
    UINT cpuAccessFlag;

    if(!dynamic)
    {
        usage = D3D11_USAGE_IMMUTABLE;
        cpuAccessFlag = 0;
    }
    else if(cpuWrite)
    {
        usage = D3D11_USAGE_DYNAMIC;
        cpuAccessFlag = D3D11_CPU_ACCESS_WRITE;
    }
    else
    {
        usage = D3D11_USAGE_DEFAULT;
        cpuAccessFlag = 0;
    }

    return CreateD3D11Buffer(gDevice, byteSize, initData, D3D11_BIND_CONSTANT_BUFFER, usage, cpuAccessFlag, 0, 0);
}

ID3D11Texture2D *CreateTexture2D(
    const D3D11_TEXTURE2D_DESC &desc, const D3D11_SUBRESOURCE_DATA *initData)
{
    ID3D11Texture2D *texture;
    HRESULT hr = gDevice->CreateTexture2D(&desc, initData, &texture);
    return SUCCEEDED(hr) ? texture : nullptr;
}

ID3D11ShaderResourceView *CreateShaderResourceView(
    const D3D11_SHADER_RESOURCE_VIEW_DESC &desc, ID3D11Resource *tex)
{
    ID3D11ShaderResourceView *srv;
    HRESULT hr = gDevice->CreateShaderResourceView(tex, &desc, &srv);
    return SUCCEEDED(hr) ? srv : nullptr;
}

VRPG_WIN_END
