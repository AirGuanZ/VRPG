#include <VRPG/Game/Block/BlockDescription.h>
#include <VRPG/Game/Block/LiquidDescription.h>
#include <VRPG/Game/Chunk/ChunkManager.h>
#include <VRPG/Game/World/BlockUpdater/LiquidUpdater.h>

VRPG_GAME_BEGIN

bool LiquidUpdater::ReactWithNeighborhood(
    const BlockInstance &block, BlockUpdaterManager &updateManager, ChunkManager &chunkManager, StdClock::time_point now)
{
    if(!block.desc->IsLiquid())
        return false;

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
                neiDiffLiquidDesc = neiDesc;
        }
    };

    updateNeiDiffLiquidDesc(PositiveX);
    updateNeiDiffLiquidDesc(NegativeX);
    updateNeiDiffLiquidDesc(PositiveZ);
    updateNeiDiffLiquidDesc(NegativeZ);
    updateNeiDiffLiquidDesc(PositiveY);

    if(!neiDiffLiquidDesc)
        return false;

    // 计算反应生成的新方块

    const LiquidDescription *liquidDesc = block.desc->GetLiquidDescription();
    bool isThisSource = liquidDesc->IsSource(*block.extraData);
    auto newBlock = block.desc->GetLiquidDescription()->ReactWith(neiDiffLiquidDesc, isThisSource, false);

    chunkManager.SetBlockID(blockPos_, newBlock.id, newBlock.orientation, std::move(newBlock.extraData));

    // 发布周围方块的更新任务

    StdClock::time_point newUpdatingTime = now + liquidDesc->spreadDelay;
    updateManager.AddUpdater(std::make_unique<LiquidUpdater>(newUpdatingTime, blockPos_ + Vec3i(+1, 0, 0)));
    updateManager.AddUpdater(std::make_unique<LiquidUpdater>(newUpdatingTime, blockPos_ + Vec3i(-1, 0, 0)));
    updateManager.AddUpdater(std::make_unique<LiquidUpdater>(newUpdatingTime, blockPos_ + Vec3i(0, 0, +1)));
    updateManager.AddUpdater(std::make_unique<LiquidUpdater>(newUpdatingTime, blockPos_ + Vec3i(0, 0, -1)));
    updateManager.AddUpdater(std::make_unique<LiquidUpdater>(newUpdatingTime, blockPos_ + Vec3i(0, +1, 0)));
    updateManager.AddUpdater(std::make_unique<LiquidUpdater>(newUpdatingTime, blockPos_ + Vec3i(0, -1, 0)));

    return true;
}

void LiquidUpdater::FlowFromNeighborhood(
    const BlockInstance &block, BlockUpdaterManager &updateManager, ChunkManager &chunkManager, StdClock::time_point now)
{
    // 排除此方块自身是液体源的情况

    if(block.desc->IsLiquid() && block.desc->GetLiquidDescription()->IsSource(*block.extraData))
        return;

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
            return;

        LiquidLevel neiLevel = ExtraDataToLiquidLevel(*nei.extraData);
        if(neiLevel <= 1)
            return;

        bool isSource = neiLevel == nei.desc->GetLiquidDescription()->sourceLevel;
        if(!isSource && chunkManager.GetBlock(neiPos + Vec3i(0, -1, 0)).desc->IsReplacable())
        {
            printf("shit\n");
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
        flowResult[int(PositiveY)].level = upDesc->GetLiquidDescription()->sourceLevel - 1;
    }

    // 按(id<, level>)对flowResult进行排序，取至多两个不同的id的结果，在具有相同id的result中取level最高的

    std::sort(std::begin(flowResult), std::end(flowResult));
    for(int i = 1, j = 0; i < 6; ++i)
    {
        if(flowResult[i].desc == flowResult[j].desc)
            flowResult[j].level = (std::max)(flowResult[i].level, flowResult[j].level);
        else
            flowResult[++j] = flowResult[i];
    }

    int resultCount = 0;
    if(flowResult[0].desc)
    {
        if(flowResult[1].desc && flowResult[1].desc != flowResult[0].desc)
            resultCount = 2;
        else
            resultCount = 1;
    }

    bool updated = false;

    if(resultCount > 1)
    {
        // 有多种液体流到此处时，取id最小的两类发生反应

        updated = true;
        auto newBlock = flowResult[0].desc->GetLiquidDescription()->ReactWith(flowResult[1].desc, false, false);
        if(BlockDescriptionManager::GetInstance().GetBlockDescription(newBlock.id)->HasExtraData())
            chunkManager.SetBlockID(blockPos_, newBlock.id, newBlock.orientation, std::move(newBlock.extraData));
        else
            chunkManager.SetBlockID(blockPos_, newBlock.id, newBlock.orientation);
    }
    else if(resultCount == 1)
    {
        // 有一种液体流到此处

        if(flowResult[0].desc != block.desc || flowResult[0].level != ExtraDataToLiquidLevel(*block.extraData))
        {
            updated = true;
            chunkManager.SetBlockID(blockPos_, flowResult[0].desc->GetBlockID(), {}, MakeLiquidExtraData(flowResult[0].level));
        }
    }
    else
    {
        // 此处应为void

        if(block.desc->GetBlockID() != BLOCK_ID_VOID)
            updated = true;
        chunkManager.SetBlockID(blockPos_, BLOCK_ID_VOID, {});
    }

    // 发布周围方块的更新任务

    if(updated)
    {
        StdClock::time_point newUpdatingTime = now + flowResult[0].desc->GetLiquidDescription()->spreadDelay;
        updateManager.AddUpdater(std::make_unique<LiquidUpdater>(newUpdatingTime, blockPos_ + Vec3i(+1, 0, 0)));
        updateManager.AddUpdater(std::make_unique<LiquidUpdater>(newUpdatingTime, blockPos_ + Vec3i(-1, 0, 0)));
        updateManager.AddUpdater(std::make_unique<LiquidUpdater>(newUpdatingTime, blockPos_ + Vec3i(0, 0, +1)));
        updateManager.AddUpdater(std::make_unique<LiquidUpdater>(newUpdatingTime, blockPos_ + Vec3i(0, 0, -1)));
        updateManager.AddUpdater(std::make_unique<LiquidUpdater>(newUpdatingTime, blockPos_ + Vec3i(0, +1, 0)));
        updateManager.AddUpdater(std::make_unique<LiquidUpdater>(newUpdatingTime, blockPos_ + Vec3i(0, -1, 0)));
    }
}

LiquidUpdater::LiquidUpdater(StdClock::time_point expectedUpdatingTime, const Vec3i &blockPos) noexcept
    : BlockUpdater(expectedUpdatingTime), blockPos_(blockPos)
{
    
}

void LiquidUpdater::Execute(BlockUpdaterManager &updateManager, ChunkManager &chunkManager, StdClock::time_point now)
{
    if(blockPos_.y < 0 || blockPos_.y >= CHUNK_SIZE_Y)
        return;

    BlockInstance block = chunkManager.GetBlock(blockPos_);

    if(!block.desc->IsReplacable())
        return;

    // 处理该方块与周围的液体间发生的反应
    if(ReactWithNeighborhood(block, updateManager, chunkManager, now))
        return;

    // 处理周围液体流动到该方块处的结果，注意这里也可能发生反应
    FlowFromNeighborhood(block, updateManager, chunkManager, now);
}

VRPG_GAME_END
