#pragma once

#include <memory>
#include <queue>

#include <VRPG/Game/Common.h>

VRPG_GAME_BEGIN

class ChunkManager;
class BlockUpdateManager;

class BlockUpdater
{
    StdClock::time_point expectedUpdatingTime_;

public:

    explicit BlockUpdater(StdClock::time_point expectedUpdatingTime) noexcept
        : expectedUpdatingTime_(expectedUpdatingTime)
    {
        
    }

    virtual ~BlockUpdater() = default;

    StdClock::time_point GetExpectedUpdatingTime() const noexcept
    {
        return expectedUpdatingTime_;
    }

    virtual void Execute(BlockUpdateManager &updateManager, ChunkManager &chunkManager, StdClock::time_point now) = 0;
};

class BlockUpdateManager
{
    struct BlockUpdateComp
    {
        bool comp(const BlockUpdater *lhs, const BlockUpdater *rhs) const noexcept
        {
            assert(lhs && rhs);
            return lhs->GetExpectedUpdatingTime() > rhs->GetExpectedUpdatingTime();
        }

        bool equiv(const BlockUpdater *lhs, const BlockUpdater *rhs) const noexcept
        {
            assert(lhs && rhs);
            return lhs->GetExpectedUpdatingTime() == rhs->GetExpectedUpdatingTime();
        }
    };

    std::priority_queue<BlockUpdater*, std::vector<BlockUpdater*>, BlockUpdateComp> updaterQueue_;

    ChunkManager *chunkManager_;

public:

    explicit BlockUpdateManager(ChunkManager *chunkManager)
        : chunkManager_(chunkManager)
    {
        
    }

    ~BlockUpdateManager()
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

    void AddUpdater(std::unique_ptr<BlockUpdater> updater)
    {
        updaterQueue_.push(updater.release());
    }

    void Execute(StdClock::time_point now = StdClock::now())
    {
        while(!updaterQueue_.empty() && updaterQueue_.top()->GetExpectedUpdatingTime() <= now)
        {
            auto updater = updaterQueue_.top();
            updaterQueue_.pop();
            updater->Execute(*this, *chunkManager_, now);
            delete updater;
        }
    }
};

VRPG_GAME_END
