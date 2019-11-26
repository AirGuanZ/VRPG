#pragma once

#include <spdlog/spdlog.h>

#include <VRPG/Mesh/Mesh.h>

#define VRPG_MESH_CONVERTER_BEGIN namespace VRPG::MeshConverter {
#define VRPG_MESH_CONVERTER_END   }

VRPG_MESH_CONVERTER_BEGIN

using Vec2 = Mesh::Vec2;
using Vec3 = Mesh::Vec3;
using Vec4 = Mesh::Vec4;

using Vec2i = Mesh::Vec2i;
using Vec3i = Mesh::Vec3i;
using Vec4i = Mesh::Vec4i;

using Vec3b = Mesh::Vec3b;
using Vec4b = Mesh::Vec4b;

using Mat4   = Mesh::Mat4;
using Trans4 = Mesh::Trans4;

using Quaternion = Mesh::Quaternion;

class VRPGMeshConverterException : public std::runtime_error
{
public:

    using runtime_error::runtime_error;
};

VRPG_MESH_CONVERTER_END
