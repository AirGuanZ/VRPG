#pragma once

#include <VRPG/Game/Common.h>

VRPG_GAME_BEGIN

namespace Collision
{

    /**
     * @brief 轴对齐圆柱体
     */
    struct AACylinder
    {
        Vec3 lowCentre;
        float radius = 1;
        float height = 1;
    };

    /**
     * @brief 射线段
     */
    struct Ray
    {
        Vec3 o;
        Vec3 d;
        float t0;
        float t1;

        Ray() noexcept
            : d(1, 0, 0), t0(0), t1(1)
        {

        }
        
        Ray(const Vec3 &o, const Vec3 &d, float t0 = 0, float t1 = 1) noexcept
            : o(o), d(d), t0(t0), t1(t1)
        {

        }
    };

} // namespace Collision

VRPG_GAME_END
