#pragma once

#include <VRPG/World/Common.h>

VRPG_WORLD_BEGIN

class Camera
{
public:

    virtual ~Camera() = default;

    virtual Mat4 GetViewProjectionMatrix() const = 0;

    virtual Vec3 GetPosition() const noexcept = 0;

    virtual Vec3 GetDirection() const noexcept = 0;
};

VRPG_WORLD_END
