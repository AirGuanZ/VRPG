#pragma once

#include <stdexcept>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <VRPG/Base/Base.h>

#define VRPG_WORLD_BEGIN namespace VRPG::World {
#define VRPG_WORLD_END   }

VRPG_WORLD_BEGIN

using Base::ComPtr;

using Base::D3D::SS_VS;
using Base::D3D::SS_PS;

namespace RenderState = Base::D3D::RenderState;

using Base::D3D::BlendState;
using Base::D3D::BlendStateBuilder;
using Base::D3D::DepthState;
using Base::D3D::IndexBuffer;
using Base::D3D::InputLayout;
using Base::D3D::InputLayoutBuilder;
using Base::D3D::ConstantBuffer;
using Base::D3D::ConstantBufferSlot;
using Base::D3D::RasterizerState;
using Base::D3D::Sampler;
using Base::D3D::Shader;
using Base::D3D::ShaderResourceSlot;
using Base::D3D::ShaderResourceView;
using Base::D3D::UniformManager;
using Base::D3D::VertexBuffer;

using Base::ByteOffset;

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

class VRPGWorldException : public std::runtime_error
{
public:

    using runtime_error::runtime_error;
};

enum Direction : uint8_t
{
    PositiveX = 0,
    NegativeX = 1,
    PositiveY = 2,
    NegativeY = 3,
    PositiveZ = 4,
    NegativeZ = 5
};

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

VRPG_WORLD_END
