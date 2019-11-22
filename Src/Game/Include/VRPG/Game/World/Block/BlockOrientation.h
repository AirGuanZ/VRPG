#pragma once

#include <VRPG/Game/Common.h>

VRPG_GAME_BEGIN

/**
 * @brief 对方块朝向的编码
 *
 * X(), Y(), Z()分别表示此方块原本的+x, +y, +z方向在旋转后对应的方向，
 * 一个没有旋转过的方块的X()应等于PositiveX，Y()为PositiveY，Z()为PositiveZ
 */
class BlockOrientation
{
    uint8_t xy_;

public:

    BlockOrientation() noexcept;

    BlockOrientation(Direction posX, Direction posY) noexcept;

    Direction X() const noexcept;

    Direction Y() const noexcept;

    Direction Z() const noexcept;

    /**
     * @brief 这真的是一个合法的坐标系吗
     */
    void CheckAssertion() const noexcept;

    /**
     * @brief 将一个没旋转过的方向按此orientation旋转后的结果
     */
    Direction OriginToRotated(Direction origin) const noexcept;

    /**
     * @brief 给定一个此orientation下方向，求它在旋转前的方向
     */
    Direction RotatedToOrigin(Direction rotated) const noexcept;
};

/**
 * @brief 设unrotatedPosition是[0, 1]^3中的一个位置，返回按rotation旋转此位置后的结果
 */
inline Vec3 RotateLocalPosition(const BlockOrientation &rotation, const Vec3 &unrotatedPosition) noexcept;

inline BlockOrientation::BlockOrientation() noexcept
    : BlockOrientation(PositiveX, PositiveY)
{

}

inline BlockOrientation::BlockOrientation(Direction posX, Direction posY) noexcept
    : xy_((uint8_t(posX) << 4) | uint8_t(posY))
{

}

inline Direction BlockOrientation::X() const noexcept
{
    return Direction(xy_ >> 4);
}

inline Direction BlockOrientation::Y() const noexcept
{
    return Direction(xy_ & 0x0f);
}

inline Direction BlockOrientation::Z() const noexcept
{
    return Cross(X(), Y());
}

inline void BlockOrientation::CheckAssertion() const noexcept
{
    assert(X() != Y());
}

inline Direction BlockOrientation::OriginToRotated(Direction origin) const noexcept
{
    CheckAssertion();

    if(origin == PositiveX) return X();
    if(origin == NegativeX) return -X();
    if(origin == PositiveY) return Y();
    if(origin == NegativeY) return -Y();
    if(origin == PositiveZ) return Z();
    return -Z();
}

inline Direction BlockOrientation::RotatedToOrigin(Direction rotated) const noexcept
{
    CheckAssertion();

    if(rotated == X())  return PositiveX;
    if(rotated == -X()) return NegativeX;
    if(rotated == Y())  return PositiveY;
    if(rotated == -Y()) return NegativeY;
    if(rotated == Z())  return PositiveZ;
    return NegativeZ;
}

inline Vec3 RotateLocalPosition(const BlockOrientation &rotation, const Vec3 &unrotatedPosition) noexcept
{
    rotation.CheckAssertion();
    Vec3 ret;

    switch(rotation.X())
    {
    case PositiveX: ret.x = +unrotatedPosition.x; break;
    case PositiveY: ret.y = +unrotatedPosition.x; break;
    case PositiveZ: ret.z = +unrotatedPosition.x; break;
    case NegativeX: ret.x = -unrotatedPosition.x; break;
    case NegativeY: ret.y = -unrotatedPosition.x; break;
    case NegativeZ: ret.z = -unrotatedPosition.x; break;
    }

    switch(rotation.Y())
    {
    case PositiveX: ret.x = +unrotatedPosition.y; break;
    case PositiveY: ret.y = +unrotatedPosition.y; break;
    case PositiveZ: ret.z = +unrotatedPosition.y; break;
    case NegativeX: ret.x = -unrotatedPosition.y; break;
    case NegativeY: ret.y = -unrotatedPosition.y; break;
    case NegativeZ: ret.z = -unrotatedPosition.y; break;
    }

    switch(rotation.Z())
    {
    case PositiveX: ret.x = +unrotatedPosition.z; break;
    case PositiveY: ret.y = +unrotatedPosition.z; break;
    case PositiveZ: ret.z = +unrotatedPosition.z; break;
    case NegativeX: ret.x = -unrotatedPosition.z; break;
    case NegativeY: ret.y = -unrotatedPosition.z; break;
    case NegativeZ: ret.z = -unrotatedPosition.z; break;
    }

    return ret;
}

VRPG_GAME_END
