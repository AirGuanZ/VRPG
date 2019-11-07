#pragma once

#include <VRPG/World/Common.h>

VRPG_WORLD_BEGIN

/*
一个方块在没旋转过时的朝向+x的面称为其posXFace
       在没旋转过时的朝向+y的面称为其posYFace

在旋转过之后，可以用posXFace和posYFace旋转后的朝向来编码该方块的整体朝向
*/

class BlockOrientation
{
    Direction posXFace = PositiveX;
    Direction posYFace = PositiveY;

    uint8_t xy_;

public:

    BlockOrientation() noexcept
        : BlockOrientation(PositiveX, PositiveY)
    {
        
    }

    BlockOrientation(Direction posX, Direction posY) noexcept
        : xy_((uint8_t(posX) << 4) | uint8_t(posY))
    {
        
    }

    Direction X() const noexcept
    {
        return Direction(xy_ >> 4);
    }

    Direction Y() const noexcept
    {
        return Direction(xy_ & 0x0f);
    }

    Direction Z() const noexcept
    {
        return Cross(X(), Y());
    }

    /**
     * @brief 这真的是一个合法的坐标系吗
     */
    void CheckAssertion() const noexcept
    {
        assert(posXFace != posYFace);
    }

    /**
     * @brief 将一个没旋转过的方向按此orientation旋转后的结果
     */
    Direction OriginToRotated(Direction origin) const noexcept
    {
        CheckAssertion();

        if(origin == PositiveX)
            return X();
        if(origin == NegativeX)
            return -X();
        if(origin == PositiveY)
            return Y();
        if(origin == NegativeY)
            return -Y();
        if(origin == PositiveZ)
            return Z();
        return -Z();
    }

    /**
     * @brief 给定一个此orientation下方向，求它在旋转前的方向
     */
    Direction RotatedToOrigin(Direction rotated) const noexcept
    {
        CheckAssertion();

        if(rotated == X())
            return PositiveX;
        if(rotated == -X())
            return NegativeX;
        if(rotated == Y())
            return PositiveY;
        if(rotated == -Y())
            return NegativeY;
        if(rotated == Z())
            return PositiveZ;
        return NegativeZ;
    }
};

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

VRPG_WORLD_END
