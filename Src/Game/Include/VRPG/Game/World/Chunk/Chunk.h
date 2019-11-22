#pragma once

#include <VRPG/Game/World/Block/BlockBrightness.h>
#include <VRPG/Game/World/Block/BlockDescription.h>
#include <VRPG/Game/World/Chunk/ChunkModel.h>

VRPG_GAME_BEGIN

class ChunkBlockData
{
    BlockID          blockID_     [CHUNK_SIZE_X][CHUNK_SIZE_Z][CHUNK_SIZE_Y] = { { { 0 } } };
    BlockOrientation orientations_[CHUNK_SIZE_X][CHUNK_SIZE_Z][CHUNK_SIZE_Y];
    int              heightMap_   [CHUNK_SIZE_X][CHUNK_SIZE_Z] = { { 0 } };

    std::map<Vec3i, BlockExtraData> extraData_;

public:

    ChunkBlockData() = default;

    ChunkBlockData(const ChunkBlockData &copyFrom);

    ChunkBlockData &operator=(const ChunkBlockData &copyFrom);

    ChunkBlockData(ChunkBlockData &&moveFrom) noexcept;

    ChunkBlockData &operator=(ChunkBlockData &&moveFrom) noexcept;

    BlockID GetID(const Vec3i &blockInChunk) const noexcept;

    BlockOrientation GetOrientation(const Vec3i &blockInChunk) const noexcept;

    int GetHeight(int blockInChunkX, int blockInChunkZ) const noexcept;

    const BlockExtraData *GetExtraData(const Vec3i &blockInChunk) const;

    BlockExtraData *GetExtraData(const Vec3i &blockInChunk);

    void SetID(const Vec3i &blockInChunk, BlockID id, BlockOrientation orientation) noexcept;

    void SetID(const Vec3i &blockInChunk, BlockID id, BlockOrientation orientation, BlockExtraData extraData) noexcept;

    void SetHeight(int blockInChunkX, int blockInChunkZ, int height) noexcept;
};

class ChunkBrightnessData
{
    BlockBrightness implBlockBrightness_[CHUNK_SIZE_X][CHUNK_SIZE_Z][CHUNK_SIZE_Y];

public:

    BlockBrightness GetBrightness(const Vec3i &blockInChunk) const noexcept;

    void SetBrightness(const Vec3i &blockInChunk, BlockBrightness brightness) noexcept;
};

class Chunk
{
    ChunkPosition chunkPosition_;

    ChunkBlockData block_;
    ChunkBrightnessData brightness_;
    ChunkModel model_;

public:

    Chunk() = default;

    explicit Chunk(const ChunkPosition &chunkPosition) noexcept;

    void SetPosition(const ChunkPosition &position) noexcept;

    const ChunkPosition &GetPosition() const noexcept;

    BlockID GetID(const Vec3i &blockInChunk) const noexcept;

    BlockOrientation GetOrientation(const Vec3i &blockInChunk) const noexcept;

    BlockBrightness GetBrightness(const Vec3i &blockInChunk) const noexcept;

    int GetHeight(int blockInChunkX, int blockInChunkZ) const noexcept;

    const BlockExtraData *GetExtraData(const Vec3i &blockInChunk) const;

    BlockExtraData *GetExtraData(const Vec3i &blockInChunk);

    BlockInstance GetBlock(const Vec3i &blockInChunk) const;

    void SetID(const Vec3i &blockInChunk, BlockID id, BlockOrientation orientation) noexcept;

    void SetID(const Vec3i &blockInChunk, BlockID id, BlockOrientation orientation, BlockExtraData extraData) noexcept;

    void SetBrightness(const Vec3i &blockInChunk, BlockBrightness brightness) noexcept;

    void SetHeight(int blockInChunkX, int blockInChunkZ, int height) noexcept;

    const ChunkModel &GetChunkModel() const noexcept;

    ChunkBlockData &GetBlockData() noexcept;

    void RegenerateSectionModel(const Vec3i &sectionInChunk, const Chunk *neighboringChunks[3][3]);
};

VRPG_GAME_END

#include "./Impl/Chunk.inl"
