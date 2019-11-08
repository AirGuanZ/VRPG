#pragma once

#include <VRPG/Game/Block/BlockBrightness.h>
#include <VRPG/Game/Block/BlockDescription.h>
#include <VRPG/Game/Chunk/ChunkModel.h>

VRPG_GAME_BEGIN

class ChunkBlockData
{
    BlockID blockID_[CHUNK_SIZE_X][CHUNK_SIZE_Z][CHUNK_SIZE_Y] = { { { 0 } } };
    BlockOrientation orientations_[CHUNK_SIZE_X][CHUNK_SIZE_Z][CHUNK_SIZE_Y];
    int heightMap_[CHUNK_SIZE_X][CHUNK_SIZE_Z] = { { 0 } };

    std::map<Vec3i, BlockExtraData> extraData_;

public:

    ChunkBlockData() = default;

    ChunkBlockData(const ChunkBlockData &copyFrom)
    {
        std::memcpy(blockID_,      copyFrom.blockID_,      sizeof(blockID_));
        std::memcpy(orientations_, copyFrom.orientations_, sizeof(orientations_));
        std::memcpy(heightMap_,    copyFrom.heightMap_,    sizeof(heightMap_));
        extraData_ = copyFrom.extraData_;
    }

    ChunkBlockData &operator=(const ChunkBlockData &copyFrom)
    {
        std::memcpy(blockID_,      copyFrom.blockID_,      sizeof(blockID_));
        std::memcpy(orientations_, copyFrom.orientations_, sizeof(orientations_));
        std::memcpy(heightMap_,    copyFrom.heightMap_,    sizeof(heightMap_));
        extraData_ = copyFrom.extraData_;
        return *this;
    }

    ChunkBlockData(ChunkBlockData &&moveFrom) noexcept
    {
        std::memcpy(blockID_,      moveFrom.blockID_,      sizeof(blockID_));
        std::memcpy(orientations_, moveFrom.orientations_, sizeof(orientations_));
        std::memcpy(heightMap_,    moveFrom.heightMap_,    sizeof(heightMap_));
        extraData_ = std::move(moveFrom.extraData_);
    }

    ChunkBlockData &operator=(ChunkBlockData &&moveFrom) noexcept
    {
        std::memcpy(blockID_,      moveFrom.blockID_,      sizeof(blockID_));
        std::memcpy(orientations_, moveFrom.orientations_, sizeof(orientations_));
        std::memcpy(heightMap_,    moveFrom.heightMap_,    sizeof(heightMap_));
        extraData_ = std::move(moveFrom.extraData_);
        return *this;
    }

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

    int GetHeight(int blockInChunkX, int blockInChunkZ) const noexcept
    {
        assert(0 <= blockInChunkX && blockInChunkX < CHUNK_SIZE_X);
        assert(0 <= blockInChunkZ && blockInChunkZ < CHUNK_SIZE_Z);
        return heightMap_[blockInChunkX][blockInChunkZ];
    }

    void SetID(const Vec3i &blockInChunk, BlockID id, BlockOrientation orientation) noexcept
    {
        assert(0 <= blockInChunk.x && blockInChunk.x < CHUNK_SIZE_X);
        assert(0 <= blockInChunk.y && blockInChunk.y < CHUNK_SIZE_Y);
        assert(0 <= blockInChunk.z && blockInChunk.z < CHUNK_SIZE_Z);

        auto desc = BlockDescriptionManager::GetInstance().GetBlockDescription(id);
        if(desc->HasExtraData())
            extraData_[blockInChunk] = desc->CreateExtraData();
        else
            extraData_.erase(blockInChunk);

        blockID_[blockInChunk.x][blockInChunk.z][blockInChunk.y] = id;
        orientations_[blockInChunk.x][blockInChunk.z][blockInChunk.y] = orientation;
    }

    void SetHeight(int blockInChunkX, int blockInChunkZ, int height) noexcept
    {
        assert(0 <= blockInChunkX && blockInChunkX < CHUNK_SIZE_X);
        assert(0 <= blockInChunkZ && blockInChunkZ < CHUNK_SIZE_Z);
        heightMap_[blockInChunkX][blockInChunkZ] = height;
    }

    const BlockExtraData &GetExtraData(const Vec3i &blockInChunk) const
    {
        assert(0 <= blockInChunk.x && blockInChunk.x < CHUNK_SIZE_X);
        assert(0 <= blockInChunk.y && blockInChunk.y < CHUNK_SIZE_Y);
        assert(0 <= blockInChunk.z && blockInChunk.z < CHUNK_SIZE_Z);
        auto it = extraData_.find(blockInChunk);
        assert(it != extraData_.end());
        return it->second;
    }

    BlockExtraData &GetExtraData(const Vec3i &blockInChunk)
    {
        assert(0 <= blockInChunk.x && blockInChunk.x < CHUNK_SIZE_X);
        assert(0 <= blockInChunk.y && blockInChunk.y < CHUNK_SIZE_Y);
        assert(0 <= blockInChunk.z && blockInChunk.z < CHUNK_SIZE_Z);
        auto it = extraData_.find(blockInChunk);
        assert(it != extraData_.end());
        return it->second;
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

    const BlockExtraData &GetExtraData(const Vec3i &blockInChunk) const
    {
        return block_.GetExtraData(blockInChunk);
    }

    BlockExtraData &GetExtraData(const Vec3i &blockInChunk)
    {
        return block_.GetExtraData(blockInChunk);
    }

    BlockInstance GetBlock(const Vec3i &blockInChunk) const
    {
        assert(0 <= blockInChunk.x && blockInChunk.x < CHUNK_SIZE_X);
        assert(0 <= blockInChunk.y && blockInChunk.y < CHUNK_SIZE_Y);
        assert(0 <= blockInChunk.z && blockInChunk.z < CHUNK_SIZE_Z);
        BlockInstance ret;
        ret.desc        = BlockDescriptionManager::GetInstance().GetBlockDescription(block_.GetID(blockInChunk));
        ret.extraData   = ret.desc->HasExtraData() ? &block_.GetExtraData(blockInChunk) : nullptr;
        ret.brightness  = brightness_(blockInChunk);
        ret.orientation = block_.GetOrientation(blockInChunk);
        return ret;
    }

    const ChunkModel &GetChunkModel() const noexcept
    {
        return model_;
    }

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

    void RegenerateSectionModel(const Vec3i &sectionInChunk, const Chunk *neighboringChunks[3][3]);
};

VRPG_GAME_END
