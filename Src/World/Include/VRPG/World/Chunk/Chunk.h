#pragma once

#include <VRPG/World/Block/BlockBrightness.h>
#include <VRPG/World/Block/BlockDescription.h>
#include <VRPG/World/Chunk/ChunkModel.h>

VRPG_WORLD_BEGIN

class ChunkBlockData
{
    BlockID blockID_[CHUNK_SIZE_X][CHUNK_SIZE_Z][CHUNK_SIZE_Y] = { { { 0 } } };
    int heightMap_[CHUNK_SIZE_X][CHUNK_SIZE_Z] = { { 0 } };

public:

    BlockID GetID(int x, int y, int z) const noexcept
    {
        assert(0 <= x && x < CHUNK_SIZE_X);
        assert(0 <= y && y < CHUNK_SIZE_Y);
        assert(0 <= z && z < CHUNK_SIZE_Z);
        return blockID_[x][z][y];
    }

    void SetID(int x, int y, int z, BlockID id) noexcept
    {
        assert(0 <= x && x < CHUNK_SIZE_X);
        assert(0 <= y && y < CHUNK_SIZE_Y);
        assert(0 <= z && z < CHUNK_SIZE_Z);
        blockID_[x][z][y] = id;
    }

    int GetHeight(int blockX, int blockZ) const noexcept
    {
        return heightMap_[blockX][blockZ];
    }

    void SetHeight(int blockX, int blockZ, int height) noexcept
    {
        heightMap_[blockX][blockZ] = height;
    }
};

class ChunkBrightnessData
{
    BlockBrightness implBlockBrightness_[CHUNK_SIZE_X][CHUNK_SIZE_Z][CHUNK_SIZE_Y];

public:

    BlockBrightness &operator()(int x, int y, int z) noexcept
    {
        assert(0 <= x && x < CHUNK_SIZE_X);
        assert(0 <= y && y < CHUNK_SIZE_Y);
        assert(0 <= z && z < CHUNK_SIZE_Z);
        return implBlockBrightness_[x][z][y];
    }

    BlockBrightness operator()(int x, int y, int z) const noexcept
    {
        assert(0 <= x && x < CHUNK_SIZE_X);
        assert(0 <= y && y < CHUNK_SIZE_Y);
        assert(0 <= z && z < CHUNK_SIZE_Z);
        return implBlockBrightness_[x][z][y];
    }
};

class Chunk
{
    ChunkPosition chunkPosition_;

    ChunkBlockData block_;
    ChunkBrightnessData brightness_;
    ChunkModel model_;

    bool isDirtySinceLoaded_;

public:

    static ChunkPosition BlockToChunk(int blockX, int blockZ) noexcept
    {
        return { blockX / CHUNK_SIZE_X, blockZ / CHUNK_SIZE_Z };
    }

    static Vec3i GlobalToLocal(const Vec3i &globalBlockPosition) noexcept
    {
        return Vec3i(
            globalBlockPosition.x % CHUNK_SIZE_X,
            globalBlockPosition.y,
            globalBlockPosition.z % CHUNK_SIZE_Z);
    }

    Chunk() noexcept
        : isDirtySinceLoaded_(false)
    {
        
    }

    explicit Chunk(const ChunkPosition &chunkPosition) noexcept
        : chunkPosition_(chunkPosition), isDirtySinceLoaded_(false)
    {
        
    }

    void SetPosition(const ChunkPosition &position) noexcept
    {
        chunkPosition_ = position;
    }

    const ChunkPosition &GetPosition() const noexcept
    {
        return chunkPosition_;
    }

    BlockID GetID(int blockX, int blockY, int blockZ) const noexcept
    {
        return block_.GetID(blockX, blockY, blockZ);
    }

    void SetID(int blockX, int blockY, int blockZ, BlockID id) noexcept
    {
        block_.SetID(blockX, blockY, blockZ, id);
    }

    BlockBrightness GetBrightness(int blockX, int blockY, int blockZ) const noexcept
    {
        return brightness_(blockX, blockY, blockZ);
    }

    void SetBrightness(int blockX, int blockY, int blockZ, BlockBrightness brightness) noexcept
    {
        brightness_(blockX, blockY, blockZ) = brightness;
    }

    int GetHeight(int blockX, int blockZ) const noexcept
    {
        return block_.GetHeight(blockX, blockZ);
    }

    void SetHeight(int blockX, int blockZ, int height) noexcept
    {
        block_.SetHeight(blockX, blockZ, height);
    }

    const ChunkModel &GetChunkModel() const noexcept
    {
        return model_;
    }

    void RegenerateSectionModel(const Vec3i &sectionIndex, const Chunk *neighboringChunks[3][3]);

    bool IsDirtySinceLoaded() const noexcept
    {
        return isDirtySinceLoaded_;
    }

    void SetDirtySinceLoaded() noexcept
    {
        isDirtySinceLoaded_ = true;
    }

    ChunkBlockData &GetBlockData() noexcept
    {
        return block_;
    }
};

VRPG_WORLD_END
