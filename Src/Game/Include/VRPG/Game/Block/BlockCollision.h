#pragma once

#include <VRPG/Game/Block/BlockOrientation.h>
#include <VRPG/Game/Physics/CollisionPrimitive.h>

VRPG_GAME_BEGIN

/**
 * @brief 用于描述单个方块的碰撞体性质
 * 
 * 假设方块被置于(0, 0, 0)位置
 */
class BlockCollision
{
public:

    virtual ~BlockCollision() = default;

    /**
     * @brief 是否与给定轴对齐圆柱体发生了碰撞
     */
    virtual bool HasCollisionWith(BlockOrientation blockOrientation, const Collision::AACylinder &cylinder) const noexcept = 0;

    /**
     * @brief resolve碰撞的输出
     *
     * x, y, z轴分别对应offset[0], [1], [2]。offset[i]不为零表示在该方向上将物体移动这么长即可解除膨胀
     */
    struct ResolveCollisionResult
    {
        float axisAlignedOffset[3] = { 0, 0, 0 };
    };

    /**
     * @brief 解除与给定轴对齐圆柱体的碰撞
     * 
     * @param cylinder     移动前的圆柱体，应与此block无碰撞
     * @param newLowCentre 移动后的新圆柱体位置，可能与此block无碰撞
     * @param result       碰撞的解除方式
     * @return             是否发生了碰撞
     */
    virtual bool ResolveCollisionWith(
        BlockOrientation blockOrientation,
        const Collision::AACylinder &cylinder, const Vec3 &newLowCentre, ResolveCollisionResult *result) const noexcept = 0;

    /**
     * @brief 与射线间的求交测试
     *
     * （可选）输出与射线首先相交的面的法线
     */
    virtual bool IntersectWith(const Collision::Ray &ray, Direction *pickedFace = nullptr) const noexcept = 0;
};

VRPG_GAME_END
