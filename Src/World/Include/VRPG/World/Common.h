#pragma once

#include <stdexcept>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

#include <VRPG/Base/Base.h>

#define VRPG_WORLD_BEGIN namespace VRPG::World {
#define VRPG_WORLD_END   }

VRPG_WORLD_BEGIN

using Vec2 = Base::Vec2;
using Vec3 = Base::Vec3;
using Vec4 = Base::Vec4;

using Vec2i = Base::Vec2i;
using Vec3i = Base::Vec3i;
using Vec4i = Base::Vec4i;

class VRPGWorldException : public std::runtime_error
{
public:

    using runtime_error::runtime_error;
};

constexpr int DEFAULT_LOG_FILE_SIZE = 1048576 * 5;

VRPG_WORLD_END
