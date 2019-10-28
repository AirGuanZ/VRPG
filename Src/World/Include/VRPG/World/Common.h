#pragma once

#include <stdexcept>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <VRPG/Base/Base.h>

#define VRPG_WORLD_BEGIN namespace VRPG::World {
#define VRPG_WORLD_END   }

VRPG_WORLD_BEGIN

using Base::D3D::SS_VS;
using Base::D3D::SS_PS;

namespace RenderState = Base::D3D::RenderState;

using Base::D3D::InputLayout;
using Base::D3D::InputLayoutBuilder;
using Base::D3D::ConstantBuffer;
using Base::D3D::ConstantBufferSlot;
using Base::D3D::RasterizerState;
using Base::D3D::Shader;
using Base::D3D::UniformManager;
using Base::D3D::VertexBuffer;

using Base::ByteOffset;

using Vec2 = Base::Vec2;
using Vec3 = Base::Vec3;
using Vec4 = Base::Vec4;

using Vec2i = Base::Vec2i;
using Vec3i = Base::Vec3i;
using Vec4i = Base::Vec4i;

using Mat4 = Base::Mat4;
using Trans4 = Base::Trans4;

class VRPGWorldException : public std::runtime_error
{
public:

    using runtime_error::runtime_error;
};

constexpr int DEFAULT_LOG_FILE_SIZE = 1048576 * 5;

VRPG_WORLD_END
