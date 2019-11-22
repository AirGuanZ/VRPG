#pragma once

#include <mutex>

#include <agz/utility/container.h>
#include <agz/utility/misc.h>
#include <agz/utility/thread.h>

#include <VRPG/Game/World/Chunk/Chunk.h>

VRPG_GAME_BEGIN

/**
 * @brief 基于LRU机制管理后台缓存数据
 *
 * 除构造和析构外所有方法均为线程安全
 */
class ChunkBlockDataPool : public agz::misc::uncopyable_t
{
public:

    explicit ChunkBlockDataPool(size_t maxDataCount);

    ~ChunkBlockDataPool();

    /**
     * @brief 尝试从池子中取得指定位置的区块数据
     */
    bool GetChunkBlockData(const ChunkPosition &position, ChunkBlockData *data);

    /**
     * @brief 若池子里有该位置的数据，返回false，否则添加此数据并返回true
     */
    bool TryToAddChunkBlockData(const ChunkPosition &position, const ChunkBlockData &data);

    /**
     * @brief 向池子中添加指定位置的区块数据
     *
     * 若添加后池子大小超过maxDataCount，则会按LRU规则淘汰最近没用过的数据
     */
    void AddChunkBlockData(const ChunkPosition &position, std::unique_ptr<ChunkBlockData> &&data);

    /**
     * @brief 若池子中包含指定位置的区块数据，则对该数据执行指定操作
     *
     * 该操作不得泄露区块数据的引用
     *
     * @return 池子中是否包含了该位置的区块数据
     */
    template<typename Func>
    bool ForGivenChunkPosition(const ChunkPosition &position, Func &&func);

    /**
     * @brief 若池子中包含指定位置的区块数据，则对该数据执行指定操作
     *
     * 该操作不得泄露区块数据的引用
     *
     * @return 池子中是否包含了该位置的区块数据
     */
    template<typename Func>
    bool ForGivenChunkPosition(const ChunkPosition &position, Func &&func) const;

    /**
     * @brief 异步地试图修改池子中指定方块的id
     *
     * 若池子中没有该方块所在的区块，则此修改无效
     */
    void ModifyBlockIDInPool(const Vec3i &blockPosition, BlockID id, BlockOrientation orientation);

private:

    void DataModifierFunc();

    std::mutex mapMutex_;
    agz::container::linked_map_t<ChunkPosition, std::unique_ptr<ChunkBlockData>> map_;
    size_t maxDataCount_;

    struct DataModifyTask
    {
        Vec3i globalBlockPosition;
        BlockID newBlockID;
        BlockOrientation newBlockOrientation;
    };

    std::thread dataModifierThread_;
    agz::thread::blocking_queue_t<DataModifyTask> dataModifyTaskQueue_;
};

VRPG_GAME_END

#include "./Impl/ChunkBlockDataPool.inl"
