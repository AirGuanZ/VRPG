#include <VRPG/Game/Block/BlockDescription.h>
#include <VRPG/Game/Block/LiquidDescription.h>
#include <VRPG/Game/Chunk/ChunkManager.h>
#include <VRPG/Game/World/BlockUpdater/LiquidUpdater.h>

VRPG_GAME_BEGIN

bool LiquidUpdater::ReactWithNeighborhood(
    const BlockInstance &block, BlockUpdaterManager &updaterManager, ChunkManager &chunkManager, StdClock::time_point now)
{
    if(!block.desc->IsLiquid())
    {
        return false;
    }

    // 遍历此方块+/-x, +/-z, +y方向的方块，若它是其他类型的液体，
    // 就应当与此方块发生反应。当有多个这样的邻居方块时，取其ID最小的一个进行反应

    const BlockDescription *neiDiffLiquidDesc = nullptr;

    auto updateNeiDiffLiquidDesc = [&](Direction direction)
    {
        Vec3i neiPos = blockPos_ + DirectionToVectori(direction);
        const BlockDescription *neiDesc = chunkManager.GetBlockDesc(neiPos);
        if(neiDesc->IsLiquid() && neiDesc != block.desc)
        {
            if(!neiDiffLiquidDesc || neiDesc->GetBlockID() < neiDiffLiquidDesc->GetBlockID())
            {
                neiDiffLiquidDesc = neiDesc;
            }
        }
    };

    updateNeiDiffLiquidDesc(PositiveX);
    updateNeiDiffLiquidDesc(NegativeX);
    updateNeiDiffLiquidDesc(PositiveZ);
    updateNeiDiffLiquidDesc(NegativeZ);
    updateNeiDiffLiquidDesc(PositiveY);

    if(!neiDiffLiquidDesc)
    {
        return false;
    }

    // 计算反应生成的新方块

    const LiquidDescription *liquidDesc = block.desc->GetLiquid();
    bool isThisSource = liquidDesc->IsSource(*block.extraData);
    auto newBlock = block.desc->GetLiquid()->ReactWith(neiDiffLiquidDesc, isThisSource, false);

    chunkManager.SetBlockID(blockPos_, newBlock.id, newBlock.orientation, std::move(newBlock.extraData));

    // 发布周围方块的更新任务

    StdClock::time_point newUpdatingTime = now + liquidDesc->spreadDelay;
    updaterManager.AddUpdater(std::make_unique<LiquidUpdater>(newUpdatingTime, blockPos_ + Vec3i(+1, 0, 0)));
    updaterManager.AddUpdater(std::make_unique<LiquidUpdater>(newUpdatingTime, blockPos_ + Vec3i(-1, 0, 0)));
    updaterManager.AddUpdater(std::make_unique<LiquidUpdater>(newUpdatingTime, blockPos_ + Vec3i(0, 0, +1)));
    updaterManager.AddUpdater(std::make_unique<LiquidUpdater>(newUpdatingTime, blockPos_ + Vec3i(0, 0, -1)));
    updaterManager.AddUpdater(std::make_unique<LiquidUpdater>(newUpdatingTime, blockPos_ + Vec3i(0, +1, 0)));
    updaterManager.AddUpdater(std::make_unique<LiquidUpdater>(newUpdatingTime, blockPos_ + Vec3i(0, -1, 0)));

    return true;
}

void LiquidUpdater::FlowFromNeighborhood(
    const BlockInstance &block, BlockUpdaterManager &updaterManager, ChunkManager &chunkManager, StdClock::time_point now)
{
    // 排除此方块自身是液体源的情况

    if(block.desc->IsLiquid() && block.desc->GetLiquid()->IsSource(*block.extraData))
    {
        return;
    }

    // 计算各邻居方块流到此处的结果

    struct FlowResult
    {
        const BlockDescription *desc = nullptr;
        LiquidLevel level            = 0;

        bool operator<(const FlowResult &rhs) const noexcept
        {
            if(!desc)                                       return false;
            if(!rhs.desc)                                   return true;
            if(desc->GetBlockID() < rhs.desc->GetBlockID()) return true;
            if(desc->GetBlockID() > rhs.desc->GetBlockID()) return false;
            return level > rhs.level;
        }
    };

    FlowResult flowResult[6];

    auto flowFromHorizontalDirection = [&](Direction direction)
    {
        Vec3i neiPos = blockPos_ + DirectionToVectori(direction);
        BlockInstance nei = chunkManager.GetBlock(neiPos);
        if(!nei.desc->IsLiquid())
        {
            return;
        }

        LiquidLevel neiLevel = ExtraDataToLiquidLevel(*nei.extraData);
        if(neiLevel <= 1)
        {
            return;
        }

        bool isSource = neiLevel == nei.desc->GetLiquid()->sourceLevel;
        if(!isSource && chunkManager.GetBlock(neiPos + Vec3i(0, -1, 0)).desc->IsReplacableByLiquid())
        {
            return;
        }

        flowResult[int(direction)].desc = nei.desc;
        flowResult[int(direction)].level = isSource ? (neiLevel - 2) : (neiLevel - 1);
    };
    flowFromHorizontalDirection(PositiveX);
    flowFromHorizontalDirection(NegativeX);
    flowFromHorizontalDirection(PositiveZ);
    flowFromHorizontalDirection(NegativeZ);

    if(auto upDesc = chunkManager.GetBlockDesc(blockPos_ + Vec3i(0, 1, 0)); upDesc->IsLiquid())
    {
        flowResult[int(PositiveY)].desc = upDesc;
        flowResult[int(PositiveY)].level = upDesc->GetLiquid()->sourceLevel - 1;
    }

    // 按(id<, level>)对flowResult进行排序，取至多两个不同的id的结果，在具有相同id的result中取level最高的

    std::sort(std::begin(flowResult), std::end(flowResult));
    for(int i = 1, j = 0; i < 6; ++i)
    {
        if(flowResult[i].desc == flowResult[j].desc)
        {
            flowResult[j].level = (std::max)(flowResult[i].level, flowResult[j].level);
        }
        else
        {
            flowResult[++j] = flowResult[i];
        }
    }

    int resultCount = 0;
    if(flowResult[0].desc)
    {
        if(flowResult[1].desc && flowResult[1].desc != flowResult[0].desc)
        {
            resultCount = 2;
        }
        else
        {
            resultCount = 1;
        }
    }

    bool updated = false;
    StdClock::duration updateDelay(0);

    if(resultCount > 1)
    {
        // 有多种液体流到此处时，取id最小的两类发生反应

        updated = true;
        updateDelay = flowResult[0].desc->GetLiquid()->spreadDelay;
        auto newBlock = flowResult[0].desc->GetLiquid()->ReactWith(flowResult[1].desc, false, false);
        if(BlockDescManager::GetInstance().GetBlockDescription(newBlock.id)->HasExtraData())
        {
            chunkManager.SetBlockID(blockPos_, newBlock.id, newBlock.orientation, std::move(newBlock.extraData));
        }
        else
        {
            chunkManager.SetBlockID(blockPos_, newBlock.id, newBlock.orientation);
        }
    }
    else if(resultCount == 1)
    {
        // 有一种液体流到此处

        if(flowResult[0].desc != block.desc || flowResult[0].level != ExtraDataToLiquidLevel(*block.extraData))
        {
            updated = true;
            updateDelay = flowResult[0].desc->GetLiquid()->spreadDelay;
            chunkManager.SetBlockID(blockPos_, flowResult[0].desc->GetBlockID(), {}, MakeLiquidExtraData(flowResult[0].level));
        }
    }
    else
    {
        // 此处应为void

        if(block.desc->GetBlockID() != BLOCK_ID_VOID)
        {
            updated = true;
            chunkManager.SetBlockID(blockPos_, BLOCK_ID_VOID, {});
        }
    }

    // 发布周围方块的更新任务

    if(updated)
    {
        StdClock::time_point newUpdatingTime = now + updateDelay;
        updaterManager.AddUpdater(std::make_unique<LiquidUpdater>(newUpdatingTime, blockPos_ + Vec3i(+1, 0, 0)));
        updaterManager.AddUpdater(std::make_unique<LiquidUpdater>(newUpdatingTime, blockPos_ + Vec3i(-1, 0, 0)));
        updaterManager.AddUpdater(std::make_unique<LiquidUpdater>(newUpdatingTime, blockPos_ + Vec3i(0, 0, +1)));
        updaterManager.AddUpdater(std::make_unique<LiquidUpdater>(newUpdatingTime, blockPos_ + Vec3i(0, 0, -1)));
        updaterManager.AddUpdater(std::make_unique<LiquidUpdater>(newUpdatingTime, blockPos_ + Vec3i(0, +1, 0)));
        updaterManager.AddUpdater(std::make_unique<LiquidUpdater>(newUpdatingTime, blockPos_ + Vec3i(0, -1, 0)));
    }
}

LiquidUpdater::LiquidUpdater(StdClock::time_point expectedUpdatingTime, const Vec3i &blockPos) noexcept
    : BlockUpdater(expectedUpdatingTime), blockPos_(blockPos)
{
    
}

void LiquidUpdater::Execute(BlockUpdaterManager &updaterManager, ChunkManager &chunkManager, StdClock::time_point now)
{
    if(blockPos_.y < 0 || blockPos_.y >= CHUNK_SIZE_Y)
    {
        return;
    }

    BlockInstance block = chunkManager.GetBlock(blockPos_);

    if(!block.desc->IsReplacableByLiquid())
    {
        return;
    }

    // 处理该方块与周围的液体间发生的反应
    if(ReactWithNeighborhood(block, updaterManager, chunkManager, now))
    {
        return;
    }

    // 处理周围液体流动到该方块处的结果，注意这里也可能发生反应
    FlowFromNeighborhood(block, updaterManager, chunkManager, now);
}

void LiquidUpdater::AddUpdaterForNeighborhood(
    const Vec3i &blockPosition, BlockUpdaterManager &updaterManager, ChunkManager &chunkManager, StdClock::time_point now)
{
    auto getUpdaterDelay = [&](const Vec3i &position)
    {
        auto desc0 = chunkManager.GetBlockDesc({ position.x + 1, position.y, position.z });
        auto desc1 = chunkManager.GetBlockDesc({ position.x - 1, position.y, position.z });
        auto desc2 = chunkManager.GetBlockDesc({ position.x, position.y + 1, position.z });
        auto desc3 = chunkManager.GetBlockDesc({ position.x, position.y, position.z + 1 });
        auto desc4 = chunkManager.GetBlockDesc({ position.x, position.y, position.z - 1 });
        auto desc5 = chunkManager.GetBlockDesc(position);

        StdClock::duration minDelay = std::chrono::duration_cast<StdClock::duration>(std::chrono::milliseconds(1000000));
        if(desc0->IsLiquid()) minDelay = (std::min)(minDelay, desc0->GetLiquid()->spreadDelay);
        if(desc1->IsLiquid()) minDelay = (std::min)(minDelay, desc1->GetLiquid()->spreadDelay);
        if(desc2->IsLiquid()) minDelay = (std::min)(minDelay, desc2->GetLiquid()->spreadDelay);
        if(desc3->IsLiquid()) minDelay = (std::min)(minDelay, desc3->GetLiquid()->spreadDelay);
        if(desc4->IsLiquid()) minDelay = (std::min)(minDelay, desc4->GetLiquid()->spreadDelay);
        if(desc5->IsLiquid()) minDelay = (std::min)(minDelay, desc5->GetLiquid()->spreadDelay);

        return minDelay;
    };

    auto tryToAddUpdater = [&](const Vec3i &position)
    {
        auto delay = getUpdaterDelay(position);
        if(delay > std::chrono::duration_cast<StdClock::duration>(std::chrono::milliseconds(10000)))
        {
            return;
        }
        updaterManager.AddUpdater(std::make_unique<LiquidUpdater>(now + delay, position));
    };

    for(int dx = -1; dx <= 1; ++dx)
    {
        for(int dz = -1; dz <= 1; ++dz)
        {
            for(int dy = -1; dy <= 1; ++dy)
            {
                tryToAddUpdater({ blockPosition.x + dx, blockPosition.y + dy, blockPosition.z + dz });
            }
        }
    }
}

VRPG_GAME_END
