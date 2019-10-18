#pragma once

#include <VRPG/World/Chunk/BlockBrightness.h>
#include <VRPG/World/Chunk/BlockDescription.h>
#include <VRPG/World/Chunk/ChunkModel.h>

VRPG_WORLD_BEGIN

constexpr int CHUNK_SIZE_X = 16;
constexpr int CHUNK_SIZE_Y = 16;
constexpr int CHUNK_SIZE_Z = 16;

class Chunk
{
    BlockID implBlockID_[CHUNK_SIZE_X][CHUNK_SIZE_Z][CHUNK_SIZE_Y];
    BlockBrightness implBlockBrightness_[CHUNK_SIZE_X][CHUNK_SIZE_Z][CHUNK_SIZE_Y];

    BlockID &ID(int x, int y, int z) noexcept
    {
        assert(0 <= x && x < CHUNK_SIZE_X);
        assert(0 <= y && y < CHUNK_SIZE_Y);
        assert(0 <= z && z < CHUNK_SIZE_Z);
        return implBlockID_[x][z][y];
    }

    BlockID ID(int x, int y, int z) const noexcept
    {
        assert(0 <= x && x < CHUNK_SIZE_X);
        assert(0 <= y && y < CHUNK_SIZE_Y);
        assert(0 <= z && z < CHUNK_SIZE_Z);
        return implBlockID_[x][z][y];
    }

    BlockBrightness &Brightness(int x, int y, int z) noexcept
    {
        assert(0 <= x && x < CHUNK_SIZE_X);
        assert(0 <= y && y < CHUNK_SIZE_Y);
        assert(0 <= z && z < CHUNK_SIZE_Z);
        return implBlockBrightness_[x][z][y];
    }

    BlockBrightness Brightness(int x, int y, int z) const noexcept
    {
        assert(0 <= x && x < CHUNK_SIZE_X);
        assert(0 <= y && y < CHUNK_SIZE_Y);
        assert(0 <= z && z < CHUNK_SIZE_Z);
        return implBlockBrightness_[x][z][y];
    }

public:

    BlockID GetID(int blockX, int blockY, int blockZ) const noexcept
    {
        return ID(blockX, blockY, blockZ);
    }

    void SetID(int blockX, int blockY, int blockZ, BlockID id) noexcept
    {
        ID(blockX, blockY, blockZ) = id;
    }

    BlockBrightness GetBrightness(int blockX, int blockY, int blockZ) const noexcept
    {
        return Brightness(blockX, blockY, blockZ);
    }

    void SetBrightness(int blockX, int blockY, int blockZ, BlockBrightness brightness) noexcept
    {
        Brightness(blockX, blockY, blockZ) = brightness;
    }

    std::vector<std::shared_ptr<const ChunkModel>> GenerateModels(const Chunk neighboringChunks[6]) const;
};

VRPG_WORLD_END
