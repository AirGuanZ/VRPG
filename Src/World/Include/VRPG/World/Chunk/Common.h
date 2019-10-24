#pragma once

#include <VRPG/World/Common.h>

VRPG_WORLD_BEGIN

class Chunk;
class ChunkManager;
class ChunkModel;
class ChunkRenderer;
class PartialSectionModel;
class SectionModel;

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

VRPG_WORLD_END

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
