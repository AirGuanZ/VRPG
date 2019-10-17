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

} // namespace RenderState

VRPG_BASE_D3D_END
