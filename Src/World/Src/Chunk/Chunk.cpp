#include <VRPG/World/Block/BlockEffect.h>
#include <VRPG/World/Chunk/Chunk.h>

VRPG_WORLD_BEGIN

namespace
{
    const BlockDescription *ID2Desc(BlockID blockID) noexcept
    {
        return BlockDescriptionManager::GetInstance().GetBlockDescription(blockID);
    }
}

void Chunk::RegenerateSectionModel(const Vec3i &sectionIndex, const Chunk *neighboringChunks[4])
{
    assert(0 <= sectionIndex.x && sectionIndex.x < CHUNK_SECTION_COUNT_X);
    assert(0 <= sectionIndex.y && sectionIndex.y < CHUNK_SECTION_COUNT_Y);
    assert(0 <= sectionIndex.z && sectionIndex.z < CHUNK_SECTION_COUNT_Z);

    auto &blockEffectMgr = BlockEffectManager::GetInstance();
    auto &blockDescMgr = BlockDescriptionManager::GetInstance();

    // 准备modelBuilders

    std::vector<std::unique_ptr<PartialSectionModelBuilder>> modelBuilders;
    BlockEffectID blockEffectCount = BlockEffectID(blockEffectMgr.GetBlockEffectCount());
    modelBuilders.reserve(blockEffectCount);
    for(BlockEffectID i = 0; i < blockEffectCount; ++i)
        modelBuilders.push_back(blockEffectMgr.GetBlockEffect(i)->CreateModelBuilder());
    auto modelBuilderView = agz::misc::span(modelBuilders.data(), modelBuilders.size());

    // 遍历每个block，将其model数据追加到各自的model builder中

    Vec3i low = sectionIndex * Vec3i(CHUNK_SECTION_SIZE_X, CHUNK_SECTION_SIZE_Y, CHUNK_SECTION_SIZE_Z);
    Vec3i high = low + Vec3i(CHUNK_SECTION_SIZE_X, CHUNK_SECTION_SIZE_Y, CHUNK_SECTION_SIZE_Z);

    for(int x = low.x; x < high.x; ++x)
    {
        for(int z = low.z; z < high.z; ++z)
        {
            for(int y = low.y; y < high.y; ++y)
            {
                const BlockDescription *blockDesc = blockDescMgr.GetBlockDescription(GetID(x, y, z));
                if(!blockDesc->IsVisible())
                    continue;

                const BlockDescription *neighboringBlockDesc[6];
                neighboringBlockDesc[PositiveX] = ID2Desc(
                    x == CHUNK_SIZE_X - 1 ?
                        neighboringChunks[PositiveX]->GetID(0, y, z) :
                        GetID(x + 1, y, z));
                neighboringBlockDesc[NegativeX] = ID2Desc(
                    x == 0 ?
                        neighboringChunks[NegativeX]->GetID(CHUNK_SIZE_X - 1, y, z) :
                        GetID(x - 1, y, z));
                neighboringBlockDesc[PositiveZ] = ID2Desc(
                    z == CHUNK_SIZE_Z - 1 ?
                        neighboringChunks[PositiveZ]->GetID(x, y, 0) :
                        GetID(x, y, z + 1));
                neighboringBlockDesc[NegativeZ] = ID2Desc(
                    z == 0 ?
                        neighboringChunks[NegativeZ]->GetID(x, y, CHUNK_SIZE_Z - 1) :
                        GetID(x, y, z - 1));
                neighboringBlockDesc[PositiveY] = ID2Desc(
                    y == CHUNK_SIZE_Y - 1 ?
                        BLOCK_ID_VOID :
                        GetID(x, y + 1, z));
                neighboringBlockDesc[NegativeY] = ID2Desc(
                    y == 0 ?
                        BLOCK_ID_VOID :
                        GetID(x, y - 1, z));

                blockDesc->AddBlockModel(modelBuilderView, { x, y, z }, neighboringBlockDesc);
            }
        }
    }

    // 用modelBuilders创建新的sectionModel，取代原来的

    auto newSectionModel = std::unique_ptr<SectionModel>();
    for(auto &builder : modelBuilders)
    {
        if(auto model = builder->Build())
            newSectionModel->partialModels.push_back(std::move(model));
    }
    model_.sectionModel(sectionIndex) = std::move(newSectionModel);
}

VRPG_WORLD_END
