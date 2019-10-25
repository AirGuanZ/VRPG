#pragma once

#include <mutex>
#include <thread>

#include <VRPG/World/Chunk/ChunkBlockDataPool.h>
#include <VRPG/World/Chunk/ChunkLoaderTask.h>
#include <VRPG/World/Land/LandGenerator.h>

/*
Chunk Streaming Task分为加载和卸载两类
    每个ChunkPosition被固定hash到一个线程上，
    因此每个位置上的区块的加载和卸载是串行完成的，
    这避免了不同线程对同一区块的加载、卸载产生的数据一致性问题
*/

VRPG_WORLD_BEGIN

/**
 * @brief 区块加载、卸载任务调度器
 * 
 * 除initialize和destroy外，所有公开接口均为线程安全
 */
class ChunkLoader
{
    struct PerThreadData
    {
        ChunkLoaderTaskQueue taskQueue;
    };

    std::vector<std::thread> threads_;
    std::unique_ptr<PerThreadData[]> perThreadData_;

    std::unique_ptr<ChunkBlockDataPool> blockDataPool_;
    std::unique_ptr<LandGenerator> landGenerator_;

    std::mutex loadingResultsMutex_;
    std::unique_ptr<std::queue<std::unique_ptr<Chunk>>> loadingResults_;

    std::atomic<bool> skipLoading_;

    std::shared_ptr<spdlog::logger> log_;

public:

    ChunkLoader();

    ~ChunkLoader();

    /**
     * @param threadCount 后台加载/卸载线程的数量
     * @param poolSize 区块池容量，一般来说应略大于区块加载范围的外几层区块数量
     * @param landGenerator 地形生成器
     */
    void Initialize(int threadCount, int poolSize, std::unique_ptr<LandGenerator> landGenerator);

    bool IsAvailable() const noexcept;

    void Destroy();

    /**
     * @brief 添加加载指定位置的区块的任务
     *
     * 加载得到的区块会被自动放置到GetALlLoadingResults的返回元素中
     */
    void AddLoadingTask(const ChunkPosition &position);

    /**
     * @brief 添加卸载指定位置的区块的任务
     */
    void AddUnloadingTask(std::unique_ptr<Chunk> &&chunk);

    /**
     * @brief 取得所有已加载的区块数据
     *
     * 注意调用该方法会移交这些数据的所有权，调用后ChunkLoader内的loadingResults队列将被清空
     */
    std::vector<std::unique_ptr<Chunk>> GetAllLoadingResults();

    /**
     * @brief 添加一个加载完成的区块数据
     */
    std::unique_ptr<Chunk> LoadChunk(const ChunkPosition &position);

    /**
     * @brief 尝试设置缓存池中指定位置的方块的id，返回true当且仅当池子中包含该方块的数据
     */
    bool SetChunkBlockDataInPool(int blockX, int blockY, int blockZ, BlockID id);
	
private:

    void LoadChunkBlockData(const ChunkPosition &position, ChunkBlockData *blockData);
    
    void AddLoadingResult(std::unique_ptr<Chunk> &&loadedChunk);

    static void WorkerFunc(ChunkLoader *chunkLoader, PerThreadData *threadLocalData);

    void ExecuteTask(ChunkLoaderTask &&task);
};

VRPG_WORLD_END
