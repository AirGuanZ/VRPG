#pragma once

#include <cassert>
#include <condition_variable>
#include <mutex>
#include <queue>

#include <agz/utility/misc.h>

#include <VRPG/World/Chunk/Chunk.h>

VRPG_WORLD_BEGIN

struct ChunkLoaderTask_Load
{
    ChunkPosition position;
    std::unique_ptr<Chunk> chunk;
};

struct ChunkLoaderTask_Unload
{
    std::unique_ptr<Chunk> chunk;
};

struct ChunkLoaderTask_Stop
{
    
};

using ChunkLoaderTask = agz::misc::variant_t<ChunkLoaderTask_Load, ChunkLoaderTask_Unload, ChunkLoaderTask_Stop>;

/**
 * @brief 区块加载任务队列
 *
 * 需支持以下操作
 * - 添加一个新的加载任务到末端
 * - 添加一个新的卸载任务到末端
 * - 添加一个结束标记到末端
 * - 弹出首个任务
 *
 * 同位置相邻任务简化：
 * - 加载 加载 => 取消其中一个加载
 * - 卸载 加载 => 把卸载的东西bypass给加载
 * - 加载 卸载 => 把卸载的东西bypass给加载
 * - 卸载 卸载 => 取消前一个卸载
 *
 * 所有操作均是线程安全的
 */
class ChunkLoaderTaskQueue
{
    std::mutex mutex_;
    std::condition_variable condVar_;

    std::queue<ChunkPosition> queue_;
    std::map<ChunkPosition, ChunkLoaderTask> map_;

    // lhs和rhs都必须是加载/卸载
    // stop类task拥有特别的key，绝不会需要和其他task合并
    static ChunkLoaderTask MergeTasks(ChunkLoaderTask &&lhs, ChunkLoaderTask &&rhs)
    {
        assert(lhs.is<ChunkLoaderTask_Load>() || lhs.is<ChunkLoaderTask_Unload>());
        assert(rhs.is<ChunkLoaderTask_Load>() || rhs.is<ChunkLoaderTask_Unload>());

        if(auto lhs_load = lhs.as_if<ChunkLoaderTask_Load>())
        {
            if(rhs.is<ChunkLoaderTask_Load>())
            {
                // 加载-加载，此时取消前一个加载
                return std::move(lhs);
            }

            // 加载-卸载，此时把卸载内容bypass给加载
            auto rhs_unload = &rhs.as<ChunkLoaderTask_Unload>();
            return ChunkLoaderTask(ChunkLoaderTask_Load{ lhs_load->position, std::move(rhs_unload->chunk) });
        }

        auto lhs_unload = &lhs.as<ChunkLoaderTask_Unload>();
        if(auto rhs_load = rhs.as_if<ChunkLoaderTask_Load>())
        {
            // 卸载-加载，此时把卸载内容bypass给加载
            return ChunkLoaderTask(ChunkLoaderTask_Load{ rhs_load->position, std::move(lhs_unload->chunk) });
        }

        // 卸载-卸载，此时取消前一个卸载
        return std::move(rhs);
    }

public:

    void AddLoadingTask(const ChunkPosition &position)
    {
        AGZ_SCOPE_GUARD({ condVar_.notify_one(); });
        std::lock_guard lk(mutex_);

        auto it = map_.find(position);
        if(it == map_.end())
        {
            queue_.push(position);
            map_[position] = ChunkLoaderTask(ChunkLoaderTask_Load{ position, nullptr });
            return;
        }

        it->second = MergeTasks(std::move(it->second), ChunkLoaderTask(ChunkLoaderTask_Load{ position, nullptr }));
    }

    void AddUnloadingTask(std::unique_ptr<Chunk> chunk)
    {
        AGZ_SCOPE_GUARD({ condVar_.notify_one(); });
        std::lock_guard lk(mutex_);

        ChunkPosition position = chunk->GetPosition();
        auto it = map_.find(position);
        if(it == map_.end())
        {
            queue_.push(position);
            map_[position] = ChunkLoaderTask(ChunkLoaderTask_Unload{ std::move(chunk) });
            return;
        }

        it->second = MergeTasks(std::move(it->second), ChunkLoaderTask(ChunkLoaderTask_Unload{ std::move(chunk) }));
    }

    void Stop()
    {
        AGZ_SCOPE_GUARD({ condVar_.notify_one(); });
        std::lock_guard lk(mutex_);

        ChunkPosition position = {
            (std::numeric_limits<int>::max)(),
            (std::numeric_limits<int>::max)()
        };
        queue_.push(position);
        map_[position] = ChunkLoaderTask(ChunkLoaderTask_Stop{});
    }

    ChunkLoaderTask GetTask()
    {
        std::unique_lock lk(mutex_);

        while(queue_.empty())
            condVar_.wait(lk);

        ChunkPosition position = queue_.front();
        queue_.pop();
        auto it = map_.find(position);
        assert(it != map_.end());
        auto ret = std::move(it->second);
        map_.erase(it);
        return ret;
    }
};

VRPG_WORLD_END
