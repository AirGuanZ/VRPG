#pragma once

#include <VRPG/Game/Common.h>

VRPG_GAME_BEGIN

/**
 * @brief 射线与[0, 1]^3立方体间的相交测试
 */
inline bool RayIntersectStdBox(const Vec3 &start, const Vec3 &dir, float minT, float maxT, Direction *pickedFace) noexcept
{
    Vec3 invDir = dir.map([](float v) { return 1 / v; });

    float nx = invDir[0] * (-start[0]);
    float ny = invDir[1] * (-start[1]);
    float nz = invDir[2] * (-start[2]);

    float fx = invDir[0] * (1 - start[0]);
    float fy = invDir[1] * (1 - start[1]);
    float fz = invDir[2] * (1 - start[2]);

    float tMin = (std::max)(minT, (std::min)(nx, fx));
    tMin       = (std::max)(tMin, (std::min)(ny, fy));
    tMin       = (std::max)(tMin, (std::min)(nz, fz));

    float tMax = (std::min)(maxT, (std::max)(nx, fx));
    tMax       = (std::min)(tMax, (std::max)(ny, fy));
    tMax       = (std::min)(tMax, (std::max)(nz, fz));

    if(pickedFace)
    {
        Vec3 inct = start + tMin * dir;
        Direction face = PositiveX; float dis = std::abs(1 - inct.x);
        auto update = [&](float newDis, Direction newFace)
        {
            if(newDis < dis)
            {
                dis = newDis;
                face = newFace;
            }
        };
        update(std::abs(inct.x),         NegativeX);
        update(std::abs(1 - inct.y), PositiveY);
        update(std::abs(inct.y),         NegativeY);
        update(std::abs(1 - inct.z), PositiveZ);
        update(std::abs(inct.z),         NegativeZ);
        *pickedFace = face;
    }

    return tMin <= tMax;
}

VRPG_GAME_END
