#include <VRPG/Mesh/MeshComponent.h>
#include <VRPG/Mesh/Utility.h>

VRPG_MESH_BEGIN

void MeshComponent::Write(std::ostream &out) const
{
    uint32_t vertexCount = static_cast<uint32_t>(vertices.size());
    WriteOStream(out, vertexCount);
    WriteOStream(out, vertices.data(), sizeof(vertices[0]) * vertices.size());

    uint32_t indexCount = static_cast<uint32_t>(indices.size());
    WriteOStream(out, indexCount);
    WriteOStream(out, indices.data(), sizeof(indices[0]) * indices.size());
}

void MeshComponent::WriteText(std::ostream &out) const
{
    
}

void MeshComponent::Read(std::istream &in)
{
    vertices.clear();
    indices.clear();

    uint32_t vertexCount;
    ReadIStream(in, vertexCount);

    vertices.resize(vertexCount);
    ReadIStream(in, vertices.data(), sizeof(vertices[0]) * vertices.size());

    uint32_t indexCount;
    ReadIStream(in, indexCount);

    indices.resize(indexCount);
    ReadIStream(in, indices.data(), sizeof(indices[0]) * indices.size());
}

VRPG_MESH_END
