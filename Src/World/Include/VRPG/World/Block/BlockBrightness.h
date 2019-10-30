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
    
    光源亮度的最大传播距离不得超过一个chunk
*/

VRPG_WORLD_BEGIN

struct BlockBrightness
{
    uint8_t r = 0; // red
    uint8_t g = 0; // green
    uint8_t b = 0; // blue
    uint8_t s = 0; // sky
};

static_assert(sizeof(BlockBrightness) == sizeof(uint32_t));
static_assert(alignof(BlockBrightness) <= alignof(uint32_t));

/**
 * @brief elementwise max
 */
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

constexpr BlockBrightness BLOCK_BRIGHTNESS_MIN = { 0, 0, 0, 0 };
constexpr BlockBrightness BLOCK_BRIGHTNESS_MAX = { 255, 255, 255, 255 };
constexpr BlockBrightness BLOCK_BRIGHTNESS_SKY = { 0, 0, 0, 15 };

/**
 * @brief 将block brightness的单个分量映射到[0, 1]的范围内
 */
inline float BlockBrightnessToFloat(uint8_t brightness) noexcept
{
    return (std::min)(brightness, uint8_t(15)) / 15.0f;
}

/**
 * @brief 将block brightness映射到[0, 1]^4的范围内
 */
inline Vec4 BlockBrightnessToFloat(BlockBrightness brightness) noexcept
{
    return Vec4(
        BlockBrightnessToFloat(brightness.r),
        BlockBrightnessToFloat(brightness.g),
        BlockBrightnessToFloat(brightness.b),
        BlockBrightnessToFloat(brightness.s));
}

/**
 * @brief 将四个block brightness映射为一份vertex brightness
 */
inline Vec4 BlockAO(const Vec4 &a, const Vec4 &b, const Vec4 &c, const Vec4 &d) noexcept
{
    return 0.15f + 0.85f * 0.25f * (a + b + c + d);
}

VRPG_WORLD_END
