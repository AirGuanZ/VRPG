#include <VRPG/Game/World/Block/BlockEffect.h>
#include <VRPG/Game/World/Chunk/Chunk.h>

VRPG_GAME_BEGIN

void Chunk::RegenerateSectionModel(const Vec3i &sectionInChunk, const Chunk *neighboringChunks[3][3])
{
    assert(0 <= sectionInChunk.x && sectionInChunk.x < CHUNK_SECTION_COUNT_X);
    assert(0 <= sectionInChunk.y && sectionInChunk.y < CHUNK_SECTION_COUNT_Y);
    assert(0 <= sectionInChunk.z && sectionInChunk.z < CHUNK_SECTION_COUNT_Z);
    assert(neighboringChunks[1][1] == this);

    auto &blockDescMgr = BlockDescManager::GetInstance();

    // 准备modelBuilders

    Vec3i globalSectionPosition = {
        chunkPosition_.x * CHUNK_SECTION_COUNT_X + sectionInChunk.x,
        sectionInChunk.y,
        chunkPosition_.z * CHUNK_SECTION_COUNT_Z + sectionInChunk.z
    };
    ModelBuilderSet modelBuilders(globalSectionPosition);

    // 遍历每个block，将其model数据追加到各自的model builder中

    Vec3i lowBlockInChunk = sectionInChunk * Vec3i(CHUNK_SECTION_SIZE_X, CHUNK_SECTION_SIZE_Y, CHUNK_SECTION_SIZE_Z);
    Vec3i highBlockInChunk = lowBlockInChunk + Vec3i(CHUNK_SECTION_SIZE_X, CHUNK_SECTION_SIZE_Y, CHUNK_SECTION_SIZE_Z);

    auto voidDesc = blockDescMgr.GetBlockDescription(BLOCK_ID_VOID);
    auto getBlock = [&](int x, int y, int z)
    {
        if(y < 0 || y >= CHUNK_SIZE_Y)
        {
            return BlockInstance{ voidDesc, nullptr, BLOCK_BRIGHTNESS_MIN, BlockOrientation() };
        }
        auto [ckPos, blkPos] = DecomposeGlobalBlockByChunk({ x, y, z });
        return neighboringChunks[ckPos.x][ckPos.z]->GetBlock(blkPos);
    };

    // 给定方块坐标，找出它周围3*3*3的方块的信息，方块本身存放在[1][1][1]中
    auto fillNeighbors = [&](
        int x, int y, int z,
        BlockNeighborhood neighborhood)
    {
        --x, --y, --z;
        for(int lx = 0; lx <= 2; ++lx)
        {
            for(int ly = 0; ly <= 2; ++ly)
            {
                for(int lz = 0; lz <= 2; ++lz)
                {
                    neighborhood[lx][ly][lz] = getBlock(x + lx, y + ly, z + lz);
                }
            }
        }
    };

    int xBase = chunkPosition_.x * CHUNK_SIZE_X;
    int zBase = chunkPosition_.z * CHUNK_SIZE_Z;

    BlockNeighborhood neighborhood;

    for(int x = lowBlockInChunk.x; x < highBlockInChunk.x; ++x)
    {
        for(int z = lowBlockInChunk.z; z < highBlockInChunk.z; ++z)
        {
            for(int y = lowBlockInChunk.y; y < highBlockInChunk.y; ++y)
            {
                if(!blockDescMgr.GetBlockDescription(GetID({ x, y, z }))->IsVisible())
                {
                    continue;
                }

                fillNeighbors(x + CHUNK_SIZE_X, y, z + CHUNK_SIZE_Z, neighborhood);
                neighborhood[1][1][1].desc->AddBlockModel(modelBuilders, { xBase + x, y, zBase + z }, neighborhood);
            }
        }
    }

    // 用modelBuilders创建新的sectionModel，取代原来的

    auto newSectionModel = std::make_unique<SectionModel>();
    for(auto &builder : modelBuilders)
    {
        if(auto model = builder->Build())
            newSectionModel->partialModels.push_back(std::move(model));
    }
    model_.sectionModel(sectionInChunk) = std::move(newSectionModel);
}

VRPG_GAME_END
