#pragma once

#include <VRPG/Base/Common.h>

VRPG_BASE_D3D_BEGIN

namespace RenderState
{

    inline void Draw(D3D11_PRIMITIVE_TOPOLOGY topology, UINT vertexCount, UINT startVertex = 0)
    {
        gDeviceContext->IASetPrimitiveTopology(topology);
        gDeviceContext->Draw(vertexCount, startVertex);
    }

    inline void DrawIndexed(D3D11_PRIMITIVE_TOPOLOGY topology, UINT indexCount, UINT startIndex = 0, UINT baseVertex = 0)
    {
        gDeviceContext->IASetPrimitiveTopology(topology);
        gDeviceContext->DrawIndexed(indexCount, startIndex, baseVertex);
    }

} // namespace RenderState

VRPG_BASE_D3D_END
