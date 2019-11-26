#pragma once

#include <VRPG/Mesh/Mesh.h>

VRPG_GAME_BEGIN

template<typename Vertex, typename Index>
class AnimatedSubMesh
{
public:

    void Initialize(
        const Vertex *vertexData, UINT vertexCount, const Index *indexData, UINT indexCount)
    {
        vertexBuffer_.Initialize(vertexCount, false, vertexData);
        indexBuffer_.Initialize(indexCount, false, indexData);
    }

    void Render() const
    {
        vertexBuffer_.Bind(0);
        indexBuffer_.Bind();
        RenderState::DrawIndexed(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, indexBuffer_.GetIndexCount());
        indexBuffer_.Unbind();
        vertexBuffer_.Unbind(0);
    }

private:

    VertexBuffer<Vertex> vertexBuffer_;
    IndexBuffer<Index>   indexBuffer_;
};

VRPG_GAME_END
