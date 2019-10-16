#pragma once

#include <VRPG/Base/D3D/D3DCreate.h>

VRPG_BASE_D3D_BEGIN

template<typename Vertex>
class VertexBuffer
{
    ComPtr<ID3D11Buffer> buffer_;

public:

    void Initialize(size_t vertexCount, bool dynamic, const Vertex *initData)
    {
        D3D11_USAGE usage;
        UINT cpuAccessFlag;
        if(dynamic)
        {
            usage = D3D11_USAGE_DYNAMIC;
            cpuAccessFlag = D3D11_CPU_ACCESS_WRITE;
        }
        else
        {
            if(!initData)
                throw VRPGBaseException("initData is nullptr for immutable vertex buffer");
            usage = D3D11_USAGE_IMMUTABLE;
            cpuAccessFlag = 0;
        }
        
        D3D11_BUFFER_DESC bufferDesc;
        bufferDesc.Usage               = usage;
        bufferDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.ByteWidth           = UINT(sizeof(Vertex) * vertexCount);
        bufferDesc.CPUAccessFlags      = cpuAccessFlag;
        bufferDesc.MiscFlags           = 0;
        bufferDesc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA subrscData;
        subrscData.pSysMem          = initData;
        subrscData.SysMemPitch      = 0;
        subrscData.SysMemSlicePitch = 0;

        buffer_ = CreateD3D11Buffer(bufferDesc, initData ? &subrscData : nullptr);
        if(!buffer_)
            throw VRPGBaseException("failed to create vertex buffer");
    }

    bool IsAvailable() const noexcept
    {
        return buffer_ != nullptr;
    }

    void Destroy()
    {
        buffer_.Reset();
    }

    ID3D11Buffer *Get() const noexcept
    {
        return buffer_.Get();
    }

    ID3D11Buffer *const *GetAddressOf() const noexcept
    {
        return buffer_.GetAddressOf();
    }

    operator ID3D11Buffer*() const noexcept
    {
        return buffer_.Get();
    }

    void SetValue(size_t start, size_t count, const Vertex *data)
    {
        D3D11_BOX box;
        box.left   = UINT(start * sizeof(Vertex));
        box.right  = box.left + UINT(count * sizeof(Vertex));
        box.top    = 0;
        box.bottom = 1;
        box.front  = 0;
        box.back   = 1;
        gDeviceContext->UpdateSubresource(buffer_.Get(), 0, &box, data, 0, 0);
    }
};

VRPG_BASE_D3D_END
