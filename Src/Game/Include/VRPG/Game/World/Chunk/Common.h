#pragma once

#include <VRPG/Game/Common.h>

VRPG_GAME_BEGIN

class Chunk;
class ChunkManager;
class ChunkModel;
class ChunkRenderer;
class PartialSectionModel;
class ModelBuilderSet;
class SectionModel;

/**
 * Chunk是管理BlockData的基本单位，一个Chunk由多个Section构成，Section是管理RenderModel的基本单位
 */

constexpr int CHUNK_SECTION_SIZE_X = 16;
constexpr int CHUNK_SECTION_SIZE_Y = 16;
constexpr int CHUNK_SECTION_SIZE_Z = 16;

constexpr int CHUNK_SECTION_COUNT_X = 2;
constexpr int CHUNK_SECTION_COUNT_Y = 16;
constexpr int CHUNK_SECTION_COUNT_Z = 2;

constexpr int CHUNK_SIZE_X = CHUNK_SECTION_SIZE_X * CHUNK_SECTION_COUNT_X;
constexpr int CHUNK_SIZE_Y = CHUNK_SECTION_SIZE_Y * CHUNK_SECTION_COUNT_Y;
constexpr int CHUNK_SIZE_Z = CHUNK_SECTION_SIZE_Z * CHUNK_SECTION_COUNT_Z;

struct ChunkPosition
{
    int x = 0;
    int z = 0;

    bool operator<(const ChunkPosition &rhs) const noexcept
    {
        return x < rhs.x || (x == rhs.x && z < rhs.z);
    }

    bool operator==(const ChunkPosition &rhs) const noexcept
    {
        return x == rhs.x && z == rhs.z;
    }
};

namespace Impl
{
    constexpr int BLOCK_POSITION_PAD = 0x40000000;
}

/**
 * @brief 将世界坐标系中的Block位置分解为它所属的Chunk位置以及它在Chunk中的位置
 */
inline std::pair<ChunkPosition, Vec3i> DecomposeGlobalBlockByChunk(const Vec3i &globalBlock) noexcept
{
    ChunkPosition ckPos = {
        (globalBlock.x + Impl::BLOCK_POSITION_PAD) / CHUNK_SIZE_X - Impl::BLOCK_POSITION_PAD / CHUNK_SIZE_X,
        (globalBlock.z + Impl::BLOCK_POSITION_PAD) / CHUNK_SIZE_Z - Impl::BLOCK_POSITION_PAD / CHUNK_SIZE_Z
    };
    Vec3i blockInChunk = {
        globalBlock.x - ckPos.x * CHUNK_SIZE_X,
        globalBlock.y,
        globalBlock.z - ckPos.z * CHUNK_SIZE_Z
    };
    return { ckPos, blockInChunk };
}

/**
 * @brief 将世界坐标系中的Block位置转换为它在所属的Chunk中的位置
 */
inline Vec3i GlobalBlockToBlockInChunk(const Vec3i &globalBlock) noexcept
{
    return {
        (globalBlock.x + Impl::BLOCK_POSITION_PAD) % CHUNK_SIZE_X,
        globalBlock.y,
        (globalBlock.z + Impl::BLOCK_POSITION_PAD) % CHUNK_SIZE_Z
    };
}

/**
 * @brief 将世界坐标系中的Block位置转换为它所属的Chunk位置
 */
inline ChunkPosition GlobalBlockToChunk(int globalBlockX, int globalBlockZ) noexcept
{
    return {
        (globalBlockX + Impl::BLOCK_POSITION_PAD) / CHUNK_SIZE_X - Impl::BLOCK_POSITION_PAD / CHUNK_SIZE_X,
        (globalBlockZ + Impl::BLOCK_POSITION_PAD) / CHUNK_SIZE_Z - Impl::BLOCK_POSITION_PAD / CHUNK_SIZE_Z
    };
}

/**
 * @brief 将世界坐标系中的Block位置转换为它所属的Chunk位置
 */
inline ChunkPosition GlobalBlockToChunk(const Vec3i &globalBlock) noexcept
{
    return GlobalBlockToChunk(globalBlock.x, globalBlock.z);
}

/**
 * @brief 将世界坐标系中的Block位置转换为它所属的Section在世界坐标系中的位置
 */
inline Vec3i GlobalBlockToGlobalSection(const Vec3i &globalBlock) noexcept
{
    return {
        (globalBlock.x + Impl::BLOCK_POSITION_PAD) / CHUNK_SECTION_SIZE_X - Impl::BLOCK_POSITION_PAD / CHUNK_SECTION_SIZE_X,
        (globalBlock.y + Impl::BLOCK_POSITION_PAD) / CHUNK_SECTION_SIZE_Y - Impl::BLOCK_POSITION_PAD / CHUNK_SECTION_SIZE_Y,
        (globalBlock.z + Impl::BLOCK_POSITION_PAD) / CHUNK_SECTION_SIZE_Z - Impl::BLOCK_POSITION_PAD / CHUNK_SECTION_SIZE_Z
    };
}

/**
 * @brief 将世界坐标系中的Block位置转换为它在所属的Section中的位置
 */
inline Vec3i GlobalBlockToBlockInSection(const Vec3i &globalBlock) noexcept
{
    return {
        (globalBlock.x + Impl::BLOCK_POSITION_PAD) % CHUNK_SECTION_SIZE_X,
        (globalBlock.y + Impl::BLOCK_POSITION_PAD) % CHUNK_SECTION_SIZE_Y,
        (globalBlock.z + Impl::BLOCK_POSITION_PAD) % CHUNK_SECTION_SIZE_Z
    };
}

/**
 * @brief 将世界坐标系中的Section位置分解为它所属的Chunk位置以及它在该Chunk中的位置
 */
inline std::pair<ChunkPosition, Vec3i> DecomposeGlobalSectionByChunk(const Vec3i &globalSection) noexcept
{
    ChunkPosition ckPos = {
        (globalSection.x + Impl::BLOCK_POSITION_PAD) / CHUNK_SECTION_COUNT_X - Impl::BLOCK_POSITION_PAD / CHUNK_SECTION_COUNT_X,
        (globalSection.z + Impl::BLOCK_POSITION_PAD) / CHUNK_SECTION_COUNT_Z - Impl::BLOCK_POSITION_PAD / CHUNK_SECTION_COUNT_Z
    };
    Vec3i sectionInChunk = {
        globalSection.x - ckPos.x * CHUNK_SECTION_COUNT_X,
        globalSection.y,
        globalSection.z - ckPos.z * CHUNK_SECTION_COUNT_Z
    };
    return { ckPos, sectionInChunk };
}

/**
 * @brief 将世界坐标系中的Section位置转换为它在所属Chunk中的位置
 */
inline Vec3i GlobalSectionToSectionInChunk(const Vec3i &globalSection) noexcept
{
    return {
        (globalSection.x + Impl::BLOCK_POSITION_PAD) % CHUNK_SECTION_COUNT_X,
        globalSection.y,
        (globalSection.z + Impl::BLOCK_POSITION_PAD) % CHUNK_SECTION_COUNT_Z
    };
}

VRPG_GAME_END

namespace std
{
    template<>
    struct hash<VRPG::World::ChunkPosition>
    {
        size_t operator()(const VRPG::World::ChunkPosition &ckPos) const noexcept
        {
            return agz::misc::hash(ckPos.x, ckPos.z);
        }
    };
}
