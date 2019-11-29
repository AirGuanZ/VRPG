#pragma once

#include <chrono>
#include <stdexcept>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <VRPG/Base/Base.h>

#define VRPG_GAME_BEGIN namespace VRPG::World {
#define VRPG_GAME_END   }

VRPG_GAME_BEGIN

using Base::ComPtr;

// Vertex Shader && Pixel Shader

using Base::D3D::SS_VS;
using Base::D3D::SS_PS;

namespace RenderState = Base::D3D::RenderState;

using Base::D3D::BlendState;
using Base::D3D::BlendStateBuilder;
using Base::D3D::DepthState;
using Base::D3D::Immediate2D;
using Base::D3D::IndexBuffer;
using Base::D3D::InputLayout;
using Base::D3D::InputLayoutBuilder;
using Base::D3D::ConstantBuffer;
using Base::D3D::ConstantBufferSlot;
using Base::D3D::RasterizerState;
using Base::D3D::Sampler;
using Base::D3D::SamplerSlot;
using Base::D3D::Shader;
using Base::D3D::ShaderResourceSlot;
using Base::D3D::ShaderResourceView;
using Base::D3D::UniformManager;
using Base::D3D::VertexBuffer;

using Vec2 = Base::Vec2;
using Vec3 = Base::Vec3;
using Vec4 = Base::Vec4;

using Vec2i = Base::Vec2i;
using Vec3i = Base::Vec3i;
using Vec4i = Base::Vec4i;

using Vec3b = Base::Vec3b;
using Vec4b = Base::Vec4b;

using Mat4 = Base::Mat4;
using Trans4 = Base::Trans4;

/**
 * @brief 全局使用的Wall Clock类型
 */
using StdClock = std::chrono::high_resolution_clock;

/**
 * @brief 全局基本异常类
 */
class VRPGGameException : public std::runtime_error
{
public:

    using runtime_error::runtime_error;
};

/**
 * @brief 轴对齐的六个方向
 */
enum Direction : uint8_t
{
    PositiveX = 0,
    NegativeX = 1,
    PositiveY = 2,
    NegativeY = 3,
    PositiveZ = 4,
    NegativeZ = 5
};

/**
 * @brief 取一个方向的反方向
 */
inline Direction operator-(Direction origin) noexcept
{
    static const Direction table[6] =
    {
        NegativeX, PositiveX,
        NegativeY, PositiveY,
        NegativeZ, PositiveZ
    };
    return table[int(origin)];
}

/**
 * @brief 将方向映射到其对应的方向向量
 */
inline const Vec3i DirectionToVectori(Direction dir) noexcept
{
    static const Vec3i offset[] =
    {
        Vec3i(1, 0, 0), Vec3i(-1, 0, 0),
        Vec3i(0, 1, 0), Vec3i(0, -1, 0),
        Vec3i(0, 0, 1), Vec3i(0, 0, -1)
    };
    return offset[int(dir)];
}

/**
 * @brief 将方向映射到其对应的方向向量
 */
inline const Vec3 DirectionToVectorf(Direction dir) noexcept
{
    static const Vec3 offset[] =
    {
        Vec3(1, 0, 0), Vec3(-1, 0, 0),
        Vec3(0, 1, 0), Vec3(0, -1, 0),
        Vec3(0, 0, 1), Vec3(0, 0, -1)
    };
    return offset[int(dir)];
}

inline bool IsPositive(Direction dir) noexcept
{
    return (uint8_t(dir) & 1) == 0;
}

inline Direction Cross(Direction lhs, Direction rhs) noexcept
{
    static const Direction table[6][6] =
    {
        {
            /* +x, +x */ PositiveY,
            /* +x, -x */ NegativeY,
            /* +x, +y */ PositiveZ,
            /* +x, -y */ NegativeZ,
            /* +x, +z */ NegativeY,
            /* +x, -z */ PositiveY
        },
        {
            /* -x, +x */ NegativeY,
            /* -x, -x */ PositiveY,
            /* -x, +y */ NegativeZ,
            /* -x, -y */ PositiveZ,
            /* -x, +z */ PositiveY,
            /* -x, -z */ NegativeY
        },
        {
            /* +y, +x */ NegativeZ,
            /* +y, -x */ PositiveZ,
            /* +y, +y */ PositiveZ,
            /* +y, -y */ NegativeZ,
            /* +y, +z */ PositiveX,
            /* +y, -z */ NegativeX
        },
        {
            /* -y, +x */ PositiveZ,
            /* -y, -x */ NegativeZ,
            /* -y, +y */ NegativeZ,
            /* -y, -y */ PositiveZ,
            /* -y, +z */ NegativeX,
            /* -y, -z */ PositiveX
        },
        {
            /* +z, +x */ PositiveY,
            /* +z, -x */ NegativeY,
            /* +z, +y */ NegativeX,
            /* +z, -y */ PositiveX,
            /* +z, +z */ PositiveY,
            /* +z, -z */ NegativeY
        },
        {
            /* +-z, +x */ NegativeY,
            /* +-z, -x */ PositiveY,
            /* +-z, +y */ PositiveX,
            /* +-z, -y */ NegativeX,
            /* +-z, +z */ NegativeY,
            /* +-z, -z */ PositiveY
        }
    };
    return table[int(lhs)][int(rhs)];
}

VRPG_GAME_END
