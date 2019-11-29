#pragma once

#include <ostream>

#include <VRPG/Mesh/Common.h>

VRPG_MESH_BEGIN

struct MeshVertex
{
    Vec3 position;
    Vec3 normal;
    Vec2 uv;
};

struct MeshComponent
{
    std::vector<MeshVertex> vertices;
    std::vector<uint32_t>   indices;

    void Write(std::ostream &out) const;

    void WriteText(std::ostream &out) const;

    void Read(std::istream &in);

    void ReadText(std::istream &in);
};

VRPG_MESH_END
