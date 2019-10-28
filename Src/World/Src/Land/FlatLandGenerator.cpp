#include <VRPG/World/Block/BlockDescription.h>
#include <VRPG/World/Land/FlatLandGenerator.h>

VRPG_WORLD_BEGIN

FlatLandGenerator::FlatLandGenerator(int landHeight) noexcept
    : landHeight_(landHeight)
{
    
}

void FlatLandGenerator::Generate(const ChunkPosition &position, ChunkBlockData *blockData)
{
    auto defaultBlockID = BlockDescriptionManager::GetInstance().GetBlockDescriptionByName("default")->GetBlockID();

    for(int x = 0; x < CHUNK_SIZE_X; ++x)
    {
        for(int z = 0; z < CHUNK_SIZE_Z; ++z)
        {
            //int height = (x + z) / 4 + 1;
            int height = landHeight_;
            if(x == 5 && z == 5)
                ++height;

            blockData->SetHeight(x, z, height);
            for(int y = 0; y <= height; ++y)
            {
                blockData->SetID(x, y, z, defaultBlockID);
            }
        }
    }
}

VRPG_WORLD_END
