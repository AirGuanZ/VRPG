#pragma once

#include <VRPG/Game/World/Block/BlockCollision.h>

VRPG_GAME_BEGIN

/**
 * @brief 无碰撞体积方块对应的block collision实现
 */
class VoidBlockCollision : public BlockCollision
{
public:

    bool HasCollisionWith(BlockOrientation blockOrientation, const Collision::AACylinder &cylinder) const noexcept override;

    bool ResolveCollisionWith(
        BlockOrientation blockOrientation,
        const Collision::AACylinder &cylinder, const Vec3 &newLowCentre, ResolveCollisionResult *result) const noexcept override;

    bool IntersectWith(const Collision::Ray &ray, Direction *pickedFace) const noexcept override;
};

VRPG_GAME_END
