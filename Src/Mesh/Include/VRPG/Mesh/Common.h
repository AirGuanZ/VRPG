#pragma once

#include <VRPG/Base/Base.h>

#define VRPG_MESH_BEGIN namespace VRPG::Mesh {
#define VRPG_MESH_END   }

VRPG_MESH_BEGIN

using Vec2 = Base::Vec2;
using Vec3 = Base::Vec3;
using Vec4 = Base::Vec4;

using Vec2i = Base::Vec2i;
using Vec3i = Base::Vec3i;
using Vec4i = Base::Vec4i;

using Vec3b = Base::Vec3b;
using Vec4b = Base::Vec4b;

using Mat4   = Base::Mat4;
using Trans4 = Base::Trans4;

using Quaternion = Base::Quaternion;

class VRPGMeshException : public std::runtime_error
{
public:

    using runtime_error::runtime_error;
};

VRPG_MESH_END
