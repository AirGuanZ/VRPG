#include <VRPG/Game/World/Block/BasicCollision/VoidCollision.h>

VRPG_GAME_BEGIN

bool VoidBlockCollision::HasCollisionWith(BlockOrientation blockOrientation, const Collision::AACylinder &cylinder) const noexcept
{
    return false;
}

bool VoidBlockCollision::ResolveCollisionWith(
    BlockOrientation blockOrientation,
    const Collision::AACylinder &cylinder, const Vec3 &newLowCentre,
    ResolveCollisionResult *result) const noexcept
{
    assert(result);
    return false;
}

bool VoidBlockCollision::IntersectWith(const Collision::Ray &ray, Direction *pickedFace) const noexcept
{
    return false;
}

VRPG_GAME_END
