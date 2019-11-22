#pragma once

#include <VRPG/Game/World/Block/BlockBrightness.h>
#include <VRPG/Game/World/Block/BlockDescription.h>

VRPG_GAME_BEGIN

/**
 * @brief 计算box型方块特定顶点的亮度
 *
 * @param blocks 3x3x3的局部方块数组，其中blocks[1][1][1]为目标顶点所在的方块
 * @param vertexNormal 目标顶点的法线方向
 * @param localVertexPosition 目标定点在方块内的相对位置，一般来说在[0, 1]^3中
 */
inline Vec4 BoxVertexBrightness(const BlockNeighborhood blocks, Direction vertexNormal, const Vec3 &localVertexPosition) noexcept;

/**
 * @brief 取得box型方块上具有指定法线的面的顶点位置，要求法线方向在编译时已知
 */
template<Direction NormalDirection>
void GenerateBoxFace(Vec3 vertices[4]) noexcept;

/**
 * @brief 取得box型方块上具有指定法线的面的顶点位置，其坐标以整数（0或1）表示，要求法线方向在编译时已知
 */
template<Direction NormalDirection>
void GenerateBoxFacei(Vec3i vertices[4]) noexcept;

/**
 * @brief 取得box型方块上具有指定法线的面的顶点位置
 */
inline void GenerateBoxFaceDynamic(Direction normalDirection, Vec3 position[4]) noexcept;

/**
 * @brief 取得box型方块上具有指定法线的面的顶点位置，其坐标以整数（0或1）表示
 */
inline void GenerateBoxFaceiDynamic(Direction normalDirection, Vec3i position[4]) noexcept;

inline Vec4 BoxVertexBrightness(const BlockNeighborhood blocks, Direction vertexNormal, const Vec3 &localVertexPosition) noexcept
{
    struct FaceInfo
    {
        int faceIndex;
        int normalAxis;
    };

    static const FaceInfo FACE_INFO[] =
    {
        { 2, 0 }, // +x
        { 0, 0 }, // -x
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
    i0[normalAxis] = faceIndex, i0[sideAxis0] = 1, i0[sideAxis1] = 1;
    i1[normalAxis] = faceIndex, i1[sideAxis0] = 1 + sideOffset0, i1[sideAxis1] = 1;
    i2[normalAxis] = faceIndex, i2[sideAxis0] = 1, i2[sideAxis1] = 1 + sideOffset1;

    float brightnessRatio = vertexNormal == PositiveY ? 1 : SIDE_VERTEX_BRIGHTNESS_RATIO;

    if(blocks[i1.x][i1.y][i1.z].desc->IsFullOpaque() && blocks[i2.x][i2.y][i2.z].desc->IsFullOpaque())
    {
        return brightnessRatio * ComputeVertexBrightness(
            blocks[i0.x][i0.y][i0.z].brightness,
            blocks[i1.x][i1.y][i1.z].brightness,
            blocks[i2.x][i2.y][i2.z].brightness);
    }

    Vec3i i3;
    i3[normalAxis] = faceIndex;
    i3[sideAxis0] = 1 + sideOffset0;
    i3[sideAxis1] = 1 + sideOffset1;

    return brightnessRatio * ComputeVertexBrightness(
        blocks[i0.x][i0.y][i0.z].brightness,
        blocks[i1.x][i1.y][i1.z].brightness,
        blocks[i2.x][i2.y][i2.z].brightness,
        blocks[i3.x][i3.y][i3.z].brightness);
}

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

template<>
inline void GenerateBoxFacei<PositiveX>(Vec3i vertices[4]) noexcept
{
    vertices[0] = { 1, 0, 0 };
    vertices[1] = { 1, 1, 0 };
    vertices[2] = { 1, 1, 1 };
    vertices[3] = { 1, 0, 1 };
}

template<>
inline void GenerateBoxFacei<NegativeX>(Vec3i vertices[4]) noexcept
{
    vertices[0] = { 0, 0, 1 };
    vertices[1] = { 0, 1, 1 };
    vertices[2] = { 0, 1, 0 };
    vertices[3] = { 0, 0, 0 };
}

template<>
inline void GenerateBoxFacei<PositiveY>(Vec3i vertices[4]) noexcept
{
    vertices[0] = { 0, 1, 0 };
    vertices[1] = { 0, 1, 1 };
    vertices[2] = { 1, 1, 1 };
    vertices[3] = { 1, 1, 0 };
}

template<>
inline void GenerateBoxFacei<NegativeY>(Vec3i vertices[4]) noexcept
{
    vertices[0] = { 1, 0, 0 };
    vertices[1] = { 1, 0, 1 };
    vertices[2] = { 0, 0, 1 };
    vertices[3] = { 0, 0, 0 };
}

template<>
inline void GenerateBoxFacei<PositiveZ>(Vec3i vertices[4]) noexcept
{
    vertices[0] = { 1, 0, 1 };
    vertices[1] = { 1, 1, 1 };
    vertices[2] = { 0, 1, 1 };
    vertices[3] = { 0, 0, 1 };
}

template<>
inline void GenerateBoxFacei<NegativeZ>(Vec3i vertices[4]) noexcept
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

inline void GenerateBoxFaceiDynamic(Direction normalDirection, Vec3i position[4]) noexcept
{
    switch(normalDirection)
    {
    case PositiveX: GenerateBoxFacei<PositiveX>(position); break;
    case NegativeX: GenerateBoxFacei<NegativeX>(position); break;
    case PositiveY: GenerateBoxFacei<PositiveY>(position); break;
    case NegativeY: GenerateBoxFacei<NegativeY>(position); break;
    case PositiveZ: GenerateBoxFacei<PositiveZ>(position); break;
    case NegativeZ: GenerateBoxFacei<NegativeZ>(position); break;
    }
}

VRPG_GAME_END
