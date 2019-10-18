#include <VRPG/World/Chunk/BlockEffect.h>
#include <VRPG/World/Chunk/Chunk.h>

VRPG_WORLD_BEGIN

std::vector<std::shared_ptr<const ChunkModel>> Chunk::GenerateModels(const Chunk neighboringChunks[6]) const
{
    auto &blockEffectMgr = BlockEffectManager::GetInstance();
    auto &blockDescMgr = BlockDescriptionManager::GetInstance();

    BlockEffectID blockEffectCount = BlockEffectID(blockEffectMgr.GetBlockEffectCount());
    std::vector<std::unique_ptr<ChunkModelBuilder>> modelBuilders;
    modelBuilders.reserve(blockEffectCount);
    for(BlockEffectID i = 0; i < blockEffectCount; ++i)
        modelBuilders.push_back(blockEffectMgr.GetBlockEffect(i)->CreateModelBuilder());

    auto idToDesc = [&](BlockID id)
    {
        return blockDescMgr.GetBlockDescription(id);
    };

    for(int x = 0; x < CHUNK_SIZE_X; ++x)
    {
        for(int z = 0; z < CHUNK_SIZE_Z; ++z)
        {
            for(int y = 0; y < CHUNK_SIZE_Y; ++y)
            {
                const BlockDescription *currBlockDesc = blockDescMgr.GetBlockDescription(GetID(x, y, z));
                if(!currBlockDesc->IsVisible())
                    continue;

                const BlockDescription *neighboringBlocks[6];
                neighboringBlocks[PositiveX] = idToDesc(
                    x == CHUNK_SIZE_X - 1 ? neighboringChunks[PositiveX].GetID(0, y, z) : GetID(x + 1, y, z));
                neighboringBlocks[PositiveY] = idToDesc(
                    y == CHUNK_SIZE_Y - 1 ? neighboringChunks[PositiveY].GetID(x, 0, z) : GetID(x, y + 1, z));
                neighboringBlocks[PositiveZ] = idToDesc(
                    z == CHUNK_SIZE_Z - 1 ? neighboringChunks[PositiveZ].GetID(x, y, 0) : GetID(x, y, z + 1));
                neighboringBlocks[NegativeX] = idToDesc(
                    x == 0 ? neighboringChunks[NegativeX].GetID(CHUNK_SIZE_X - 1, y, z) : GetID(x - 1, y, z));
                neighboringBlocks[NegativeY] = idToDesc(
                    y == 0 ? neighboringChunks[NegativeY].GetID(x, CHUNK_SIZE_Y - 1, z) : GetID(x, y - 1, z));
                neighboringBlocks[NegativeZ] = idToDesc(
                    z == 0 ? neighboringChunks[NegativeZ].GetID(x, y, CHUNK_SIZE_Z - 1) : GetID(x, y, z - 1));

                currBlockDesc->AddBlockModel(
                    agz::misc::span(modelBuilders.data(), modelBuilders.size()), Vec3i(x, y, z), neighboringBlocks);
            }
        }
    }

    std::vector<std::shared_ptr<const ChunkModel>> models;
    for(auto &builder : modelBuilders)
    {
        auto model = builder->Build();
        if(model)
            models.push_back(std::move(model));
    }
    return models;
}

VRPG_WORLD_END
