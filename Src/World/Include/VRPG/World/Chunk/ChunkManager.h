#pragma once

#include <unordered_map>
#include <unordered_set>

#include <agz/utility/misc.h>

#include <VRPG/World/Chunk/Chunk.h>
#include <VRPG/World/Chunk/ChunkLoader.h>

VRPG_WORLD_BEGIN

/*
Chunk管理的中枢
    Chunk的加载-维护-渲染被分配到多个线程：
        一个渲染线程，ChunkManager不定期swap一个ChunkRenderer的指针过去，其中包含了一组immutable的ChunkModel
        一个逻辑线程，负责管理Chunk的加载/卸载任务
        一个或多个加载线程，负责创建、加载或计算新的Chunk

        加载线程在后台维护一个区块数据池，只记录block种类，不记录光照信息
        加载线程用池中的区块数据来计算新加载的区块的光照
        逻辑线程更新某个区块的内容时，也要通知加载线程同步改变其区块池中的内容（如果有的话）

        每个区块依照其位置被分配给固定的线程，这使得对同一个区块的加载和卸载工作实质上是串行的，避免读写数据的一致性问题
*/

class ChunkRenderer;

struct ChunkManagerParams
{
    // renderDistance < loadDistance < unloadDistance
    // <= renderDistance -> render
    // <= loadDistance   -> load
    // >  unloadDistance -> destroy
    int renderDistance;
    int loadDistance;
    int unloadDistance;

    // 后台负责区块加载/卸载线程的数量
    int backgroundThreadCount;
    // 后台区块数据池的大小
    int backgroundPoolSize;
};

/**
 * @brief 区块管理设施
 *
 * 除非特殊指明，该设施的所有公开接口都只能从区块管理线程调用
 */
class ChunkManager : public agz::misc::uncopyable_t
{
public:

    ChunkManager(const ChunkManagerParams &params, std::unique_ptr<LandGenerator> landGenerator);

    /**
     * @brief 设置中心区块的位置，这可能会产生一系列的区块加载和卸载
     */
    void SetCentreChunk(int chunkX, int chunkZ);

    /**
     * @brief 设置某个位置的block id
     *
     * 这会触发光照传播计算和渲染数据更新，必要时还会阻塞地加载该位置的区块
     */
    void SetBlockID(int blockX, int blockY, int blockZ, BlockID id);

    /**
     * @brief 取得某个位置的block id
     *
     * 必要时会阻塞地加载该位置的区块
     */
    BlockID GetBlockID(int blockX, int blockY, int blockZ);

    /**
     * @brief 取得某个位置的block brightness
     *
     * 必要时会阻塞地加载该位置的区块
     */
    BlockBrightness GetBlockBrightness(int blockX, int blockY, int blockZ);

    /**
     * @brief 和加载线程交互，获取加载完成的区块
     */
    void UpdateChunkData();

    /**
     * @brief 对有变化的区块，重新生成其渲染数据
     *
     * 有新数据被生成时返回true
     */
    bool UpdateChunkModels();

    /**
     * @brief 生成新的区块渲染队列
     */
    void FillRenderer(ChunkRenderer &renderer);

private:

    Chunk *EnsureChunkExists(int chunkX, int chunkZ);

    bool ShouldDestroy(const ChunkPosition &position) const noexcept;

    bool ShouldRender(const ChunkPosition &position) const noexcept;

    /**
     * @brief 更新blocksQueue中所有方块的光照
     *
     * 对每个方块，根据自发光、height map和与它相邻的方块的光照计算其光照
     * 若有改变，则其相邻的方块也需被加入队列更新
     *
     * 光照被改变的block所在的section会被加入sectionsWithDirtyModel_
     *
     * 此函数返回后保证blocksQueue.empty() == true
     */
    void UpdateLight(std::queue<Vec3i> &blocksQueue);

    /**
     * @brief 取得某个位置的最大高度
     *
     * 调用方需保证该方块所在的区块已经在chunks_中
     */
    int GetHeight_Unchecked(int blockX, int blockZ);

    /**
     * @brief 设置指定方块的亮度
     *
     * 调用方需保证该方块所在的区块已经在chunks_中
     */
    void SetBlockBrightness_Unchecked(int blockX, int blockY, int blockZ, BlockBrightness brightness);

    ChunkManagerParams params_;

    // 区块加载器
    std::unique_ptr<ChunkLoader> loader_;

    // 从位置到区块的映射
    std::unordered_map<ChunkPosition, std::unique_ptr<Chunk>> chunks_;

    // 哪些区块的model是陈旧的
    // 单位是section
    std::unordered_set<Vec3i> sectionsWithDirtyModel_;

    // 目前的中心区块位置
    ChunkPosition centreChunkPosition_;
};

VRPG_WORLD_END
