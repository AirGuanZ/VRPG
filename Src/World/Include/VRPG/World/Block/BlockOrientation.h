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

    /**
     * @brief 将一个没旋转过的方向按此orientation旋转后的结果
     */
    Direction OriginToRotated(Direction origin) const noexcept
    {
        if(origin == PositiveX)
            return posXFace;
        if(origin == NegativeX)
            return -posXFace;
        if(origin == PositiveY)
            return posYFace;
        if(origin == NegativeY)
            return -posYFace;
        if(origin == PositiveZ)
            return Cross(posXFace, posYFace);
        return Cross(posYFace, posXFace);
    }

    /**
     * @brief 给定一个此orientation下方向，求它在旋转前的方向
     */
    Direction RotatedToOrigin(Direction rotated) const noexcept
    {
        if(rotated == posXFace)
            return PositiveX;
        if(rotated == -posXFace)
            return NegativeX;
        if(rotated == posYFace)
            return PositiveY;
        if(rotated == -posYFace)
            return NegativeY;
        if(rotated == Cross(posXFace, posYFace))
            return PositiveZ;
        return NegativeZ;
    }
};

VRPG_WORLD_END
