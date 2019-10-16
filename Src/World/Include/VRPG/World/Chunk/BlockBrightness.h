#pragma once

#include <algorithm>

#include <VRPG/World/Common.h>

VRPG_WORLD_BEGIN

struct BlockBrightness
{
    uint8_t r ;
    uint8_t g;
    uint8_t b;
    uint8_t s;
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

VRPG_WORLD_END
