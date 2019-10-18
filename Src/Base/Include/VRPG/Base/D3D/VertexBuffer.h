#pragma once

#include <VRPG/Base/D3D/ComObjectHolder.h>
#include <VRPG/Base/D3D/D3DCreate.h>

VRPG_BASE_D3D_BEGIN

template<typename Vertex>
class VertexBuffer : public ComObjectHolder<ID3D11Buffer>
{
    UINT vertexCount_ = 0;

public:

    void Initialize(UINT vertexCount, bool dynamic, const Vertex *initData)
    {
        assert(vertexCount);

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

        obj_ = CreateD3D11Buffer(bufferDesc, initData ? &subrscData : nullptr);
        if(!obj_)
            throw VRPGBaseException("failed to create vertex buffer");
        vertexCount_ = vertexCount;
    }

    void Destroy()
    {
        obj_.Reset();
        vertexCount_ = 0;
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
        gDeviceContext->UpdateSubresource(obj_.Get(), 0, &box, data, 0, 0);
    }

    UINT GetVertexCount() const noexcept
    {
        return vertexCount_;
    }

    void Bind(UINT slot) const
    {
        UINT stride = sizeof(Vertex), offset = 0;
        gDeviceContext->IASetVertexBuffers(slot, 1, obj_.GetAddressOf(), &stride, &offset);
    }

    void Unbind(UINT slot) const
    {
        ID3D11Buffer *empty = nullptr; UINT stride = 0, offset = 0;
        gDeviceContext->IASetVertexBuffers(slot, 1, &empty, &stride, &offset);
    }
};

VRPG_BASE_D3D_END
