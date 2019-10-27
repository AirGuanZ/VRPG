#pragma once

#include <VRPG/World/Common.h>

VRPG_WORLD_BEGIN

class Camera
{
public:

    virtual ~Camera() = default;

    virtual Mat4 GetViewProjectionMatrix() const = 0;
};

VRPG_WORLD_END
