#pragma once

#include <VRPG/World/Block/BlockBrightness.h>
#include <VRPG/World/Block/BlockDescription.h>

VRPG_WORLD_BEGIN

inline Vec4 BoxVertexBrightness_X(
    const BlockDescription *description[3][3][3],
    const BlockBrightness brightness[3][3][3],
    int x, int y, int z)
{
    if(description[x][y + 1][1]->IsFullOpaque() && description[x][1][z + 1]->IsFullOpaque())
    {
        return SIDE_VERTEX_BRIGHTNESS_RATIO * ComputeVertexBrightness(
            brightness[x][1    ][1    ],
            brightness[x][1 + y][1    ],
            brightness[x][1    ][1 + z]);
    }
    return SIDE_VERTEX_BRIGHTNESS_RATIO * ComputeVertexBrightness(
        brightness[x][1    ][1    ],
        brightness[x][1 + y][1    ],
        brightness[x][1    ][1 + z],
        brightness[x][1 + y][1 + z]);
}

inline Vec4 BoxVertexBrightness_Y(
    const BlockDescription *description[3][3][3],
    const BlockBrightness brightness[3][3][3],
    int x, int y, int z)
{
    float ratio = y > 1 ? 1.0f : SIDE_VERTEX_BRIGHTNESS_RATIO;
    if(description[x + 1][y][1]->IsFullOpaque() && description[1][y][z + 1]->IsFullOpaque())
    {
        return ratio * ComputeVertexBrightness(
            brightness[1    ][y][1    ],
            brightness[1 + x][y][1    ],
            brightness[1    ][y][1 + z]);
    }
    return ratio * ComputeVertexBrightness(
        brightness[1    ][y][1    ],
        brightness[1 + x][y][1    ],
        brightness[1    ][y][1 + z],
        brightness[1 + x][y][1 + z]);
}

inline Vec4 BoxVertexBrightness_Z(
    const BlockDescription *description[3][3][3],
    const BlockBrightness brightness[3][3][3],
    int x, int y, int z)
{
    if(description[x + 1][1][z]->IsFullOpaque() && description[1][y + 1][z]->IsFullOpaque())
    {
        return SIDE_VERTEX_BRIGHTNESS_RATIO * ComputeVertexBrightness(
            brightness[1    ][1    ][z],
            brightness[1 + x][1    ][z],
            brightness[1    ][1 + y][z]);
    }
    return SIDE_VERTEX_BRIGHTNESS_RATIO * ComputeVertexBrightness(
        brightness[1    ][1    ][z],
        brightness[1 + x][1    ][z],
        brightness[1    ][1 + y][z],
        brightness[1 + x][1 + y][z]);
}

VRPG_WORLD_END
