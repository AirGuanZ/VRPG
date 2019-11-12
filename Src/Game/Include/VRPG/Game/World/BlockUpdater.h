#pragma once

#include <memory>
#include <queue>

#include <VRPG/Game/Common.h>

/*
所有的方块更新任务都有一个对应的期望执行时刻，这些任务由一个BlockUpdaterManager管理

每次调用BlockUpdaterManager，都会执行其中所有已经到了期望执行时刻的任务，按其期望时刻从早到晚的顺序进行

在执行任务的过程中，可能会产生新的任务，它们将被插入到同一个BlockUpdaterManager实例中
*/

VRPG_GAME_BEGIN

class ChunkManager;
class BlockUpdaterManager;

class BlockUpdater
{
    StdClock::time_point expectedUpdatingTime_;

public:

    explicit BlockUpdater(StdClock::time_point expectedUpdatingTime) noexcept
        : expectedUpdatingTime_(expectedUpdatingTime)
    {
        
    }

    virtual ~BlockUpdater() = default;

    virtual void Execute(BlockUpdaterManager & updateManager, ChunkManager & chunkManager, StdClock::time_point now) = 0;

    StdClock::time_point GetExpectedUpdatingTime() const noexcept
    {
        return expectedUpdatingTime_;
    }
};

class BlockUpdaterManager
{
    struct BlockUpdateComp
    {
        bool operator()(const BlockUpdater *lhs, const BlockUpdater *rhs) const noexcept
        {
            assert(lhs && rhs);
            return lhs->GetExpectedUpdatingTime() > rhs->GetExpectedUpdatingTime();
        }
    };

    std::priority_queue<BlockUpdater*, std::vector<BlockUpdater*>, BlockUpdateComp> updaterQueue_;

    ChunkManager *chunkManager_;

public:

    explicit BlockUpdaterManager(ChunkManager *chunkManager)
        : chunkManager_(chunkManager)
    {
        
    }

    ~BlockUpdaterManager()
    {
        StdClock::time_point now = StdClock::now();
        while(!updaterQueue_.empty())
        {
            auto updater = updaterQueue_.top();
            updaterQueue_.pop();
            updater->Execute(*this, *chunkManager_, now);
            delete updater;
        }
    }

    /**
     * @brief 添加一个新的方块更新任务
     */
    void AddUpdater(std::unique_ptr<BlockUpdater> updater)
    {
        updaterQueue_.push(updater.release());
    }

    /**
     * @brief 执行所有已到时间的更新任务
     */
    void Execute(int maxExecutedCount, StdClock::time_point now = StdClock::now())
    {
        int executed = 0;
        while(!updaterQueue_.empty() && updaterQueue_.top()->GetExpectedUpdatingTime() <= now)
        {
            auto updater = updaterQueue_.top();
            updaterQueue_.pop();
            updater->Execute(*this, *chunkManager_, now);
            delete updater;

            if(++executed >= maxExecutedCount)
                return;
        }
    }
};

VRPG_GAME_END
