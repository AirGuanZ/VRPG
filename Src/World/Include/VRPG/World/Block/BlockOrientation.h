#pragma once

#include <VRPG/World/Common.h>

VRPG_WORLD_BEGIN

/*
一个方块在没旋转过时的朝向+x的面称为其posXFace
       在没旋转过时的朝向+y的面称为其posYFace

在旋转过之后，可以用posXFace和posYFace旋转后的朝向来编码该方块的整体朝向
*/

struct BlockOrientation
{
    Direction posXFace = PositiveX;
    Direction posYFace = PositiveY;
    Direction posZFace = PositiveZ;

    /**
     * @brief 这真的是一个合法的坐标系吗
     */
    void CheckAssertion() const noexcept
    {
        assert(Cross(posXFace, posYFace) == posZFace);
        assert(Cross(posYFace, posZFace) == posXFace);
        assert(Cross(posZFace, posXFace) == posYFace);
        assert(Cross(posYFace, posXFace) == -posZFace);
        assert(Cross(posZFace, posYFace) == -posXFace);
        assert(Cross(posXFace, posZFace) == -posYFace);
    }

    /**
     * @brief 将一个没旋转过的方向按此orientation旋转后的结果
     */
    Direction OriginToRotated(Direction origin) const noexcept
    {
        CheckAssertion();

        if(origin == PositiveX)
            return posXFace;
        if(origin == NegativeX)
            return -posXFace;
        if(origin == PositiveY)
            return posYFace;
        if(origin == NegativeY)
            return -posYFace;
        if(origin == PositiveZ)
            return posZFace;
        return -posZFace;
    }

    /**
     * @brief 给定一个此orientation下方向，求它在旋转前的方向
     */
    Direction RotatedToOrigin(Direction rotated) const noexcept
    {
        CheckAssertion();

        if(rotated == posXFace)
            return PositiveX;
        if(rotated == -posXFace)
            return NegativeX;
        if(rotated == posYFace)
            return PositiveY;
        if(rotated == -posYFace)
            return NegativeY;
        if(rotated == posZFace)
            return PositiveZ;
        return NegativeZ;
    }
};

inline Vec3 RotateLocalPosition(const BlockOrientation &rotation, const Vec3 &unrotatedPosition) noexcept
{
    rotation.CheckAssertion();
    Vec3 ret;

    switch(rotation.posXFace)
    {
    case PositiveX: ret.x = +unrotatedPosition.x; break;
    case PositiveY: ret.y = +unrotatedPosition.x; break;
    case PositiveZ: ret.z = +unrotatedPosition.x; break;
    case NegativeX: ret.x = -unrotatedPosition.x; break;
    case NegativeY: ret.y = -unrotatedPosition.x; break;
    case NegativeZ: ret.z = -unrotatedPosition.x; break;
    }

    switch(rotation.posYFace)
    {
    case PositiveX: ret.x = +unrotatedPosition.y; break;
    case PositiveY: ret.y = +unrotatedPosition.y; break;
    case PositiveZ: ret.z = +unrotatedPosition.y; break;
    case NegativeX: ret.x = -unrotatedPosition.y; break;
    case NegativeY: ret.y = -unrotatedPosition.y; break;
    case NegativeZ: ret.z = -unrotatedPosition.y; break;
    }

    switch(Cross(rotation.posXFace, rotation.posYFace))
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
