#pragma once

#include <VRPG/World/Block/BlockBrightness.h>
#include <VRPG/World/Block/BlockDescription.h>
#include <VRPG/World/Chunk/ChunkModel.h>

VRPG_WORLD_BEGIN

class ChunkBlockData
{
    BlockID blockID_[CHUNK_SIZE_X][CHUNK_SIZE_Z][CHUNK_SIZE_Y] = { { { 0 } } };
    BlockOrientation orientations_[CHUNK_SIZE_X][CHUNK_SIZE_Z][CHUNK_SIZE_Y];
    int heightMap_[CHUNK_SIZE_X][CHUNK_SIZE_Z] = { { 0 } };

public:

    BlockID GetID(const Vec3i &blockInChunk) const noexcept
    {
        assert(0 <= blockInChunk.x && blockInChunk.x < CHUNK_SIZE_X);
        assert(0 <= blockInChunk.y && blockInChunk.y < CHUNK_SIZE_Y);
        assert(0 <= blockInChunk.z && blockInChunk.z < CHUNK_SIZE_Z);
        return blockID_[blockInChunk.x][blockInChunk.z][blockInChunk.y];
    }

    BlockOrientation GetOrientation(const Vec3i &blockInChunk) const noexcept
    {
        assert(0 <= blockInChunk.x && blockInChunk.x < CHUNK_SIZE_X);
        assert(0 <= blockInChunk.y && blockInChunk.y < CHUNK_SIZE_Y);
        assert(0 <= blockInChunk.z && blockInChunk.z < CHUNK_SIZE_Z);
        return orientations_[blockInChunk.x][blockInChunk.z][blockInChunk.y];
    }

    void SetID(const Vec3i &blockInChunk, BlockID id, BlockOrientation orientation) noexcept
    {
        assert(0 <= blockInChunk.x && blockInChunk.x < CHUNK_SIZE_X);
        assert(0 <= blockInChunk.y && blockInChunk.y < CHUNK_SIZE_Y);
        assert(0 <= blockInChunk.z && blockInChunk.z < CHUNK_SIZE_Z);
        blockID_[blockInChunk.x][blockInChunk.z][blockInChunk.y] = id;
        orientations_[blockInChunk.x][blockInChunk.z][blockInChunk.y] = orientation;
    }

    int GetHeight(int blockInChunkX, int blockInChunkZ) const noexcept
    {
        return heightMap_[blockInChunkX][blockInChunkZ];
    }

    void SetHeight(int blockInChunkX, int blockInChunkZ, int height) noexcept
    {
        heightMap_[blockInChunkX][blockInChunkZ] = height;
    }
};

class ChunkBrightnessData
{
    BlockBrightness implBlockBrightness_[CHUNK_SIZE_X][CHUNK_SIZE_Z][CHUNK_SIZE_Y];

public:

    BlockBrightness &operator()(const Vec3i &blockInChunk) noexcept
    {
        assert(0 <= blockInChunk.x && blockInChunk.x < CHUNK_SIZE_X);
        assert(0 <= blockInChunk.y && blockInChunk.y < CHUNK_SIZE_Y);
        assert(0 <= blockInChunk.z && blockInChunk.z < CHUNK_SIZE_Z);
        return implBlockBrightness_[blockInChunk.x][blockInChunk.z][blockInChunk.y];
    }

    BlockBrightness operator()(const Vec3i &blockInChunk) const noexcept
    {
        assert(0 <= blockInChunk.x && blockInChunk.x < CHUNK_SIZE_X);
        assert(0 <= blockInChunk.y && blockInChunk.y < CHUNK_SIZE_Y);
        assert(0 <= blockInChunk.z && blockInChunk.z < CHUNK_SIZE_Z);
        return implBlockBrightness_[blockInChunk.x][blockInChunk.z][blockInChunk.y];
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

    BlockID GetID(const Vec3i &blockInChunk) const noexcept
    {
        return block_.GetID(blockInChunk);
    }

    BlockOrientation GetOrientation(const Vec3i &blockInChunk) const noexcept
    {
        return block_.GetOrientation(blockInChunk);
    }

    void SetID(const Vec3i &blockInChunk, BlockID id, BlockOrientation orientation) noexcept
    {
        block_.SetID(blockInChunk, id, orientation);
    }

    BlockBrightness GetBrightness(const Vec3i &blockInChunk) const noexcept
    {
        return brightness_(blockInChunk);
    }

    void SetBrightness(const Vec3i &blockInChunk, BlockBrightness brightness) noexcept
    {
        brightness_(blockInChunk) = brightness;
    }

    int GetHeight(int blockInChunkX, int blockInChunkZ) const noexcept
    {
        return block_.GetHeight(blockInChunkX, blockInChunkZ);
    }

    void SetHeight(int blockInChunkX, int blockInChunkZ, int height) noexcept
    {
        block_.SetHeight(blockInChunkX, blockInChunkZ, height);
    }

    const ChunkModel &GetChunkModel() const noexcept
    {
        return model_;
    }

    void RegenerateSectionModel(const Vec3i &sectionInChunk, const Chunk *neighboringChunks[3][3]);

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
