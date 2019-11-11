#pragma once

VRPG_GAME_BEGIN

inline ChunkBlockData::ChunkBlockData(const ChunkBlockData &copyFrom)
{
    std::memcpy(blockID_, copyFrom.blockID_, sizeof(blockID_));
    std::memcpy(orientations_, copyFrom.orientations_, sizeof(orientations_));
    std::memcpy(heightMap_, copyFrom.heightMap_, sizeof(heightMap_));
    extraData_ = copyFrom.extraData_;
}

inline ChunkBlockData &ChunkBlockData::operator=(const ChunkBlockData &copyFrom)
{
    std::memcpy(blockID_, copyFrom.blockID_, sizeof(blockID_));
    std::memcpy(orientations_, copyFrom.orientations_, sizeof(orientations_));
    std::memcpy(heightMap_, copyFrom.heightMap_, sizeof(heightMap_));
    extraData_ = copyFrom.extraData_;
    return *this;
}

inline ChunkBlockData::ChunkBlockData(ChunkBlockData &&moveFrom) noexcept
{
    std::memcpy(blockID_, moveFrom.blockID_, sizeof(blockID_));
    std::memcpy(orientations_, moveFrom.orientations_, sizeof(orientations_));
    std::memcpy(heightMap_, moveFrom.heightMap_, sizeof(heightMap_));
    extraData_ = std::move(moveFrom.extraData_);
}

inline ChunkBlockData &ChunkBlockData::operator=(ChunkBlockData &&moveFrom) noexcept
{
    std::memcpy(blockID_, moveFrom.blockID_, sizeof(blockID_));
    std::memcpy(orientations_, moveFrom.orientations_, sizeof(orientations_));
    std::memcpy(heightMap_, moveFrom.heightMap_, sizeof(heightMap_));
    extraData_ = std::move(moveFrom.extraData_);
    return *this;
}

inline BlockID ChunkBlockData::GetID(const Vec3i &blockInChunk) const noexcept
{
    assert(0 <= blockInChunk.x && blockInChunk.x < CHUNK_SIZE_X);
    assert(0 <= blockInChunk.y && blockInChunk.y < CHUNK_SIZE_Y);
    assert(0 <= blockInChunk.z && blockInChunk.z < CHUNK_SIZE_Z);
    return blockID_[blockInChunk.x][blockInChunk.z][blockInChunk.y];
}

inline BlockOrientation ChunkBlockData::GetOrientation(const Vec3i &blockInChunk) const noexcept
{
    assert(0 <= blockInChunk.x && blockInChunk.x < CHUNK_SIZE_X);
    assert(0 <= blockInChunk.y && blockInChunk.y < CHUNK_SIZE_Y);
    assert(0 <= blockInChunk.z && blockInChunk.z < CHUNK_SIZE_Z);
    return orientations_[blockInChunk.x][blockInChunk.z][blockInChunk.y];
}

inline int ChunkBlockData::GetHeight(int blockInChunkX, int blockInChunkZ) const noexcept
{
    assert(0 <= blockInChunkX && blockInChunkX < CHUNK_SIZE_X);
    assert(0 <= blockInChunkZ && blockInChunkZ < CHUNK_SIZE_Z);
    return heightMap_[blockInChunkX][blockInChunkZ];
}

inline const BlockExtraData *ChunkBlockData::GetExtraData(const Vec3i &blockInChunk) const
{
    assert(0 <= blockInChunk.x && blockInChunk.x < CHUNK_SIZE_X);
    assert(0 <= blockInChunk.y && blockInChunk.y < CHUNK_SIZE_Y);
    assert(0 <= blockInChunk.z && blockInChunk.z < CHUNK_SIZE_Z);
    auto it = extraData_.find(blockInChunk);
    return it != extraData_.end() ? &it->second : nullptr;
}

inline BlockExtraData *ChunkBlockData::GetExtraData(const Vec3i &blockInChunk)
{
    assert(0 <= blockInChunk.x && blockInChunk.x < CHUNK_SIZE_X);
    assert(0 <= blockInChunk.y && blockInChunk.y < CHUNK_SIZE_Y);
    assert(0 <= blockInChunk.z && blockInChunk.z < CHUNK_SIZE_Z);
    auto it = extraData_.find(blockInChunk);
    return it != extraData_.end() ? &it->second : nullptr;
}

inline void ChunkBlockData::SetID(const Vec3i &blockInChunk, BlockID id, BlockOrientation orientation) noexcept
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

inline void ChunkBlockData::SetID(const Vec3i &blockInChunk, BlockID id, BlockOrientation orientation, BlockExtraData extraData) noexcept
{
    assert(0 <= blockInChunk.x && blockInChunk.x < CHUNK_SIZE_X);
    assert(0 <= blockInChunk.y && blockInChunk.y < CHUNK_SIZE_Y);
    assert(0 <= blockInChunk.z && blockInChunk.z < CHUNK_SIZE_Z);

    auto desc = BlockDescriptionManager::GetInstance().GetBlockDescription(id);
    if(desc->HasExtraData())
        extraData_[blockInChunk] = std::move(extraData);
    else
        extraData_.erase(blockInChunk);

    blockID_[blockInChunk.x][blockInChunk.z][blockInChunk.y] = id;
    orientations_[blockInChunk.x][blockInChunk.z][blockInChunk.y] = orientation;
}

inline void ChunkBlockData::SetHeight(int blockInChunkX, int blockInChunkZ, int height) noexcept
{
    assert(0 <= blockInChunkX && blockInChunkX < CHUNK_SIZE_X);
    assert(0 <= blockInChunkZ && blockInChunkZ < CHUNK_SIZE_Z);
    heightMap_[blockInChunkX][blockInChunkZ] = height;
}

inline BlockBrightness ChunkBrightnessData::GetBrightness(const Vec3i &blockInChunk) const noexcept
{
    assert(0 <= blockInChunk.x && blockInChunk.x < CHUNK_SIZE_X);
    assert(0 <= blockInChunk.y && blockInChunk.y < CHUNK_SIZE_Y);
    assert(0 <= blockInChunk.z && blockInChunk.z < CHUNK_SIZE_Z);
    return implBlockBrightness_[blockInChunk.x][blockInChunk.z][blockInChunk.y];
}

inline void ChunkBrightnessData::SetBrightness(const Vec3i &blockInChunk, BlockBrightness brightness) noexcept
{
    assert(0 <= blockInChunk.x && blockInChunk.x < CHUNK_SIZE_X);
    assert(0 <= blockInChunk.y && blockInChunk.y < CHUNK_SIZE_Y);
    assert(0 <= blockInChunk.z && blockInChunk.z < CHUNK_SIZE_Z);
    implBlockBrightness_[blockInChunk.x][blockInChunk.z][blockInChunk.y] = brightness;
}

inline Chunk::Chunk(const ChunkPosition &chunkPosition) noexcept
    : chunkPosition_(chunkPosition)
{

}

inline void Chunk::SetPosition(const ChunkPosition &position) noexcept
{
    chunkPosition_ = position;
}

inline const ChunkPosition &Chunk::GetPosition() const noexcept
{
    return chunkPosition_;
}

inline BlockID Chunk::GetID(const Vec3i &blockInChunk) const noexcept
{
    return block_.GetID(blockInChunk);
}

inline BlockOrientation Chunk::GetOrientation(const Vec3i &blockInChunk) const noexcept
{
    return block_.GetOrientation(blockInChunk);
}

inline BlockBrightness Chunk::GetBrightness(const Vec3i &blockInChunk) const noexcept
{
    return brightness_.GetBrightness(blockInChunk);
}

inline int Chunk::GetHeight(int blockInChunkX, int blockInChunkZ) const noexcept
{
    return block_.GetHeight(blockInChunkX, blockInChunkZ);
}

inline const BlockExtraData *Chunk::GetExtraData(const Vec3i &blockInChunk) const
{
    return block_.GetExtraData(blockInChunk);
}

inline BlockExtraData *Chunk::GetExtraData(const Vec3i &blockInChunk)
{
    return block_.GetExtraData(blockInChunk);
}

inline BlockInstance Chunk::GetBlock(const Vec3i &blockInChunk) const
{
    assert(0 <= blockInChunk.x && blockInChunk.x < CHUNK_SIZE_X);
    assert(0 <= blockInChunk.y && blockInChunk.y < CHUNK_SIZE_Y);
    assert(0 <= blockInChunk.z && blockInChunk.z < CHUNK_SIZE_Z);
    BlockInstance ret;
    ret.desc = BlockDescriptionManager::GetInstance().GetBlockDescription(block_.GetID(blockInChunk));
    ret.extraData = ret.desc->HasExtraData() ? block_.GetExtraData(blockInChunk) : nullptr;
    ret.brightness = brightness_.GetBrightness(blockInChunk);
    ret.orientation = block_.GetOrientation(blockInChunk);
    return ret;
}

inline void Chunk::SetID(const Vec3i &blockInChunk, BlockID id, BlockOrientation orientation) noexcept
{
    block_.SetID(blockInChunk, id, orientation);
}

inline void Chunk::SetID(const Vec3i &blockInChunk, BlockID id, BlockOrientation orientation, BlockExtraData extraData) noexcept
{
    block_.SetID(blockInChunk, id, orientation, std::move(extraData));
}

inline void Chunk::SetBrightness(const Vec3i &blockInChunk, BlockBrightness brightness) noexcept
{
    brightness_.SetBrightness(blockInChunk, brightness);
}

inline void Chunk::SetHeight(int blockInChunkX, int blockInChunkZ, int height) noexcept
{
    block_.SetHeight(blockInChunkX, blockInChunkZ, height);
}

inline const ChunkModel &Chunk::GetChunkModel() const noexcept
{
    return model_;
}

inline ChunkBlockData &Chunk::GetBlockData() noexcept
{
    return block_;
}

VRPG_GAME_END
