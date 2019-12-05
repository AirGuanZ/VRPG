#pragma once

#include <VRPG/Game/World/Block/BlockCollision.h>

VRPG_GAME_BEGIN

class BoxBlockCollision : public BlockCollision
{
    bool enableCollision_;

public:

    BoxBlockCollision(bool enableCollision = true);

    bool HasCollisionWith(BlockOrientation blockOrientation, const Collision::AACylinder &cylinder) const noexcept override;

    bool ResolveCollisionWith(
        BlockOrientation blockOrientation,
        const Collision::AACylinder &cylinder, const Vec3 &newLowCentre, ResolveCollisionResult *result) const noexcept override;

    bool IntersectWith(const Collision::Ray &ray, Direction *pickedFace) const noexcept override;
};

VRPG_GAME_END
