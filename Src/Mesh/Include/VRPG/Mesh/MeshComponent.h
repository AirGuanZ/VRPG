#pragma once

#include <VRPG/Mesh/Common.h>

VRPG_MESH_BEGIN

// 静态网格物体

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
};

VRPG_MESH_END
