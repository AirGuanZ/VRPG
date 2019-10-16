#include <VRPG/World/Chunk/BlockEffect.h>
#include <VRPG/World/Chunk/Chunk.h>

VRPG_WORLD_BEGIN

std::vector<std::unique_ptr<ChunkModel>> Chunk::GenerateModels(const Chunk neighboringChunks[6]) const
{
    auto &blockEffectMgr = BlockEffectManager::GetInstance();

    size_t blockEffectCount = blockEffectMgr.GetBlockEffectCount();
    std::vector<std::unique_ptr<ChunkModelBuilder>> modelBuilders;
    modelBuilders.reserve(blockEffectCount);
    for(BlockEffectID i = 0; i < blockEffectCount; ++i)
        modelBuilders.push_back(blockEffectMgr.GetBlockEffect(i)->CreateModelBuilder());

    // TODO: build models
    // for each block
    //     find neighboring blocks
    //     block desc->build(builders, block position, neighboring blocks)

    std::vector<std::unique_ptr<ChunkModel>> models;
    for(auto &builder : modelBuilders)
    {
        auto model = builder->Build();
        if(model)
            models.push_back(std::move(model));
    }

    return models;
}

VRPG_WORLD_END
