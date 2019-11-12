#pragma once

#include <VRPG/Game/Common.h>

VRPG_GAME_BEGIN

struct CullingBoundingBox
{
    Vec3 low;
    Vec3 high;
};

class Camera
{
public:

    virtual ~Camera() = default;

    virtual Mat4 GetViewProjectionMatrix() const = 0;

    virtual Vec3 GetPosition() const noexcept = 0;

    virtual Vec3 GetDirection() const noexcept = 0;

    virtual bool IsVisible(const CullingBoundingBox &bbox) const noexcept = 0;
};

VRPG_GAME_END
