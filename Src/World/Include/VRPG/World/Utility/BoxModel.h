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

inline Vec4 BoxVertexBrightness(
    const BlockDescription *desc[3][3][3],
    const BlockBrightness brightness[3][3][3],
    Direction vertexNormal,
    const Vec3 &localVertexPosition) noexcept
{
    struct FaceInfo
    {
        int faceIndex;
        int normalAxis;
    };

    static const FaceInfo FACE_INFO[] =
    {
        { 2, 0 }, // +x
        { 0, 0 }, //-x
        { 2, 1 }, // +y
        { 0, 1 }, // -y
        { 2, 2 }, // +z
        { 0, 2 }  // -z
    };
    auto [faceIndex, normalAxis] = FACE_INFO[int(vertexNormal)];

    int sideAxis0 = (normalAxis + 1) % 3;
    int sideAxis1 = (normalAxis + 2) % 3;
    int sideOffset0 = localVertexPosition[sideAxis0] > 0 ? 1 : -1;
    int sideOffset1 = localVertexPosition[sideAxis1] > 0 ? 1 : -1;

    Vec3i i0, i1, i2;
    i0[normalAxis] = faceIndex, i0[sideAxis0] = 1,               i0[sideAxis1] = 1;
    i1[normalAxis] = faceIndex, i1[sideAxis0] = 1 + sideOffset0, i1[sideAxis1] = 1;
    i2[normalAxis] = faceIndex, i2[sideAxis0] = 1,               i2[sideAxis1] = 1 + sideOffset1;

    float brightnessRatio = vertexNormal == PositiveY ? 1 : SIDE_VERTEX_BRIGHTNESS_RATIO;

    if(desc[i1.x][i1.y][i1.z]->IsFullOpaque() && desc[i2.x][i2.y][i2.z]->IsFullOpaque())
    {
        return brightnessRatio * ComputeVertexBrightness(
            brightness[i0.x][i0.y][i0.z],
            brightness[i1.x][i1.y][i1.z],
            brightness[i2.x][i2.y][i2.z]);
    }

    Vec3i i3;
    i3[normalAxis] = faceIndex;
    i3[sideAxis0] = 1 + sideOffset0;
    i3[sideAxis1] = 1 + sideOffset1;

    return brightnessRatio * ComputeVertexBrightness(
        brightness[i0.x][i0.y][i0.z],
        brightness[i1.x][i1.y][i1.z],
        brightness[i2.x][i2.y][i2.z],
        brightness[i3.x][i3.y][i3.z]);
}

template<Direction NormalDirection>
void GenerateBoxFace(Vec3 vertices[4]) noexcept;

template<>
inline void GenerateBoxFace<PositiveX>(Vec3 vertices[4]) noexcept
{
    vertices[0] = { 1, 0, 0 };
    vertices[1] = { 1, 1, 0 };
    vertices[2] = { 1, 1, 1 };
    vertices[3] = { 1, 0, 1 };
}

template<>
inline void GenerateBoxFace<NegativeX>(Vec3 vertices[4]) noexcept
{
    vertices[0] = { 0, 0, 1 };
    vertices[1] = { 0, 1, 1 };
    vertices[2] = { 0, 1, 0 };
    vertices[3] = { 0, 0, 0 };
}

template<>
inline void GenerateBoxFace<PositiveY>(Vec3 vertices[4]) noexcept
{
    vertices[0] = { 0, 1, 0 };
    vertices[1] = { 0, 1, 1 };
    vertices[2] = { 1, 1, 1 };
    vertices[3] = { 1, 1, 0 };
}

template<>
inline void GenerateBoxFace<NegativeY>(Vec3 vertices[4]) noexcept
{
    vertices[0] = { 1, 0, 0 };
    vertices[1] = { 1, 0, 1 };
    vertices[2] = { 0, 0, 1 };
    vertices[3] = { 0, 0, 0 };
}

template<>
inline void GenerateBoxFace<PositiveZ>(Vec3 vertices[4]) noexcept
{
    vertices[0] = { 1, 0, 1 };
    vertices[1] = { 1, 1, 1 };
    vertices[2] = { 0, 1, 1 };
    vertices[3] = { 0, 0, 1 };
}

template<>
inline void GenerateBoxFace<NegativeZ>(Vec3 vertices[4]) noexcept
{
    vertices[0] = { 0, 0, 0 };
    vertices[1] = { 0, 1, 0 };
    vertices[2] = { 1, 1, 0 };
    vertices[3] = { 1, 0, 0 };
}

inline const Vec2 BOX_FACE_TEXCOORD[] =
{
    { 0, 1 },
    { 0, 0 },
    { 1, 0 },
    { 1, 1 }
};

inline void GenerateBoxFaceDynamic(Direction normalDirection, Vec3 position[4]) noexcept
{
    switch(normalDirection)
    {
    case PositiveX: GenerateBoxFace<PositiveX>(position); break;
    case NegativeX: GenerateBoxFace<NegativeX>(position); break;
    case PositiveY: GenerateBoxFace<PositiveY>(position); break;
    case NegativeY: GenerateBoxFace<NegativeY>(position); break;
    case PositiveZ: GenerateBoxFace<PositiveZ>(position); break;
    case NegativeZ: GenerateBoxFace<NegativeZ>(position); break;
    }
}

VRPG_WORLD_END
