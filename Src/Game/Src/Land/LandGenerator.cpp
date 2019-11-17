#include <VRPG/Game/Land/LandGenerator.h>

VRPG_GAME_BEGIN

void LandGenerator::ComputeHeightMap(ChunkBlockData *blockData) noexcept
{
    for(int x = 0; x < CHUNK_SIZE_X; ++x)
    {
        for(int z = 0; z < CHUNK_SIZE_Z; ++z)
        {
            int height = CHUNK_SIZE_Y - 1;
            while(height > 0 && blockData->GetID({ x, height, z }) == BLOCK_ID_VOID)
            {
                --height;
            }
            blockData->SetHeight(x, z, height);
        }
    }
}

VRPG_GAME_END
