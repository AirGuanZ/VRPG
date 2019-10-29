#include <VRPG/World/Block/BlockEffect.h>
#include <VRPG/World/Chunk/Chunk.h>

VRPG_WORLD_BEGIN

void Chunk::RegenerateSectionModel(const Vec3i &sectionIndex, const Chunk *neighboringChunks[3][3])
{
    assert(0 <= sectionIndex.x && sectionIndex.x < CHUNK_SECTION_COUNT_X);
    assert(0 <= sectionIndex.y && sectionIndex.y < CHUNK_SECTION_COUNT_Y);
    assert(0 <= sectionIndex.z && sectionIndex.z < CHUNK_SECTION_COUNT_Z);
    assert(neighboringChunks[1][1] == this);

    auto &blockDescMgr = BlockDescriptionManager::GetInstance();

    // 准备modelBuilders

    PartialSectionModelBuilderSet modelBuilders;

    // 遍历每个block，将其model数据追加到各自的model builder中

    Vec3i low = sectionIndex * Vec3i(CHUNK_SECTION_SIZE_X, CHUNK_SECTION_SIZE_Y, CHUNK_SECTION_SIZE_Z);
    Vec3i high = low + Vec3i(CHUNK_SECTION_SIZE_X, CHUNK_SECTION_SIZE_Y, CHUNK_SECTION_SIZE_Z);

    auto voidDesc = blockDescMgr.GetBlockDescription(BLOCK_ID_VOID);
    auto getDescAndBrightness = [&](int x, int y, int z)
    {
        if(y < 0 || y >= CHUNK_SIZE_Y)
            return std::make_pair(voidDesc, BLOCK_BRIGHTNESS_MIN);

        int ckX = x / CHUNK_SIZE_X;
        int ckZ = z / CHUNK_SIZE_Z;
        int blkX = x % CHUNK_SIZE_X;
        int blkY = y;
        int blkZ = z % CHUNK_SIZE_Z;

        BlockID id = neighboringChunks[ckX][ckZ]->GetID(blkX, blkY, blkZ);
        BlockBrightness brightness = neighboringChunks[ckX][ckZ]->GetBrightness(blkX, blkY, blkZ);
        return std::make_pair(blockDescMgr.GetBlockDescription(id), brightness);
    };

    auto fillNeighbors = [&](
        int x, int y, int z,
        const BlockDescription *neighbors[3][3][3],
        BlockBrightness brightness[3][3][3])
    {
        --x, --y, --z;
        for(int lx = 0; lx <= 2; ++lx)
        {
            for(int ly = 0; ly <= 2; ++ly)
            {
                for(int lz = 0; lz <= 2; ++lz)
                {
                    auto [desc, light] = getDescAndBrightness(x + lx, y + ly, z + lz);
                    neighbors[lx][ly][lz] = desc;
                    brightness[lx][ly][lz] = light;
                }
            }
        }
    };

    int xBase = chunkPosition_.x * CHUNK_SIZE_X;
    int zBase = chunkPosition_.z * CHUNK_SIZE_Z;

    for(int x = low.x; x < high.x; ++x)
    {
        for(int z = low.z; z < high.z; ++z)
        {
            for(int y = low.y; y < high.y; ++y)
            {
                const BlockDescription *neighborDescs[3][3][3];
                BlockBrightness neighborBrightness[3][3][3];
                fillNeighbors(x + CHUNK_SIZE_X, y, z + CHUNK_SIZE_Z, neighborDescs, neighborBrightness);
                neighborDescs[1][1][1]->AddBlockModel(modelBuilders, { xBase + x, y, zBase + z }, neighborDescs, neighborBrightness);
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
    model_.sectionModel(sectionIndex) = std::move(newSectionModel);
}

VRPG_WORLD_END
