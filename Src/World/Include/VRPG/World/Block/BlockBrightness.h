#pragma once

#include <algorithm>

#include <VRPG/World/Common.h>

/**
方块亮度模型：
    对任意方块，设
        旁边的方块的最大亮度为(nr, ng, nb, ns)
        自身的光照衰减为(ar, ag, ab, as)
        自发光为(er, eg, eb, es) (es = 0)
        直接天光为(dr, dg, db, ds) (dr = dg = db = 0)
    则该方块的亮度为
        (max(er, nr - ar, dr),
         max(eg, ng - ag, dg),
         max(eb, nb - ab, db),
         max(es, ns - as, ds))
*/

VRPG_WORLD_BEGIN

struct BlockBrightness
{
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t s = 0;
};

static_assert(sizeof(BlockBrightness) == sizeof(uint32_t));
static_assert(alignof(BlockBrightness) <= alignof(uint32_t));

inline BlockBrightness Max(const BlockBrightness &lhs, const BlockBrightness &rhs) noexcept
{
    return {
        (std::max)(lhs.r, rhs.r),
        (std::max)(lhs.g, rhs.g),
        (std::max)(lhs.b, rhs.b),
        (std::max)(lhs.s, rhs.s)
    };
}

inline bool operator==(const BlockBrightness &lhs, const BlockBrightness &rhs) noexcept
{
    return std::memcmp(&lhs, &rhs, sizeof(BlockBrightness)) == 0;
}

inline bool operator!=(const BlockBrightness &lhs, const BlockBrightness &rhs) noexcept
{
    return !(lhs == rhs);
}

inline BlockBrightness operator-(const BlockBrightness &lhs, const BlockBrightness &rhs) noexcept
{
    uint8_t new_r = lhs.r > rhs.r ? (lhs.r - rhs.r) : 0;
    uint8_t new_g = lhs.g > rhs.g ? (lhs.g - rhs.g) : 0;
    uint8_t new_b = lhs.b > rhs.b ? (lhs.b - rhs.b) : 0;
    uint8_t new_s = lhs.s > rhs.s ? (lhs.s - rhs.s) : 0;
    return BlockBrightness{ new_r, new_g, new_b, new_s };
}

constexpr BlockBrightness BLOCK_BRIGHTNESS_MAX = { 255, 255, 255, 255 };
constexpr BlockBrightness BLOCK_BRIGHTNESS_SKY = { 0, 0, 0, 15 };

VRPG_WORLD_END
