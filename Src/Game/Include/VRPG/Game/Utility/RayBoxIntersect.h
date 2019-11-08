#pragma once

#include <VRPG/Game/Common.h>

VRPG_GAME_BEGIN

/**
 * @brief 射线与[0, 1]^3立方体间的相交测试
 */
inline bool RayIntersectStdBox(const Vec3 &start, const Vec3 &invDir, float minT, float maxT) noexcept
{
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

    return tMin <= tMax;
}

VRPG_GAME_END
