#pragma once

VRPG_GAME_BEGIN

inline ChunkBlockDataPool::ChunkBlockDataPool(size_t maxDataCount)
    : maxDataCount_(maxDataCount)
{
    dataModifierThread_ = std::thread(&ChunkBlockDataPool::DataModifierFunc, this);
}

inline ChunkBlockDataPool::~ChunkBlockDataPool()
{
    dataModifyTaskQueue_.stop();
    dataModifierThread_.join();
}

inline bool ChunkBlockDataPool::GetChunkBlockData(const ChunkPosition &position, ChunkBlockData *data)
{
    std::lock_guard lk(mapMutex_);
    if(auto chunkData = map_.find_and_erase(position))
    {
        *data = **chunkData;
        map_.push_front(position, std::move(*chunkData));
        return true;
    }
    return false;
}

inline bool ChunkBlockDataPool::TryToAddChunkBlockData(const ChunkPosition &position, const ChunkBlockData &data)
{
    std::lock_guard lk(mapMutex_);
    if(map_.exists(position))
    {
        return false;
    }

    auto newData = std::make_unique<ChunkBlockData>(data);
    map_.push_front(position, std::move(newData));
    while(map_.size() > maxDataCount_)
    {
        map_.pop_back();
    }

    return true;
}

inline void ChunkBlockDataPool::AddChunkBlockData(const ChunkPosition &position, std::unique_ptr<ChunkBlockData> &&data)
{
    std::lock_guard lk(mapMutex_);
    if(map_.find_and_erase(position))
    {
        map_.push_front(position, std::move(data));
        return;
    }

    map_.push_front(position, std::move(data));
    while(map_.size() > maxDataCount_)
    {
        map_.pop_back();
    }
}

template<typename Func>
bool ChunkBlockDataPool::ForGivenChunkPosition(const ChunkPosition &position, Func &&func)
{
    std::lock_guard lk(mapMutex_);
    if(auto pChunk = map_.find(position))
    {
        func(**pChunk);
        return true;
    }
    return false;
}

template<typename Func>
bool ChunkBlockDataPool::ForGivenChunkPosition(const ChunkPosition &position, Func &&func) const
{
    std::lock_guard lk(mapMutex_);
    if(auto pChunk = map_.find(position))
    {
        func(*pChunk);
        return true;
    }
    return false;
}

inline void ChunkBlockDataPool::ModifyBlockIDInPool(const Vec3i &blockPosition, BlockID id, BlockOrientation orientation)
{
    dataModifyTaskQueue_.push({ blockPosition, id, orientation });
}

inline void ChunkBlockDataPool::DataModifierFunc()
{
    for(;;)
    {
        auto optTask = dataModifyTaskQueue_.pop_or_stop();
        if(!optTask)
        {
            break;
        }

        auto &task = *optTask;
        auto [ckPos, blkPos] = DecomposeGlobalBlockByChunk(task.globalBlockPosition);

        ForGivenChunkPosition({ ckPos.x, ckPos.z }, [&](ChunkBlockData &blockData)
        {
            blockData.SetID(blkPos, task.newBlockID, task.newBlockOrientation);

            int oldHeight = blockData.GetHeight(blkPos.x, blkPos.z);
            if(blkPos.y > oldHeight &&task.newBlockID != BLOCK_ID_VOID)
            {
                blockData.SetHeight(blkPos.x, blkPos.z, blkPos.y);
            }
            else if(blkPos.y == oldHeight && task.newBlockID == BLOCK_ID_VOID)
            {
                int newHeight = blkPos.y;
                while(newHeight >= 0 && blockData.GetID(blkPos))
                {
                    --newHeight;
                }
                blockData.SetHeight(blkPos.x, blkPos.z, newHeight);
            }
        });
    }
}

VRPG_GAME_END
