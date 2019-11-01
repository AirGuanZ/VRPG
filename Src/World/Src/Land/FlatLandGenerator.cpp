#include <VRPG/World/Block/BlockDescription.h>
#include <VRPG/World/Block/BuiltinBlock/BuiltinBlock.h>
#include <VRPG/World/Land/FlatLandGenerator.h>

VRPG_WORLD_BEGIN

FlatLandGenerator::FlatLandGenerator(int landHeight) noexcept
    : landHeight_(landHeight)
{
    
}

void FlatLandGenerator::Generate(const ChunkPosition &position, ChunkBlockData *blockData)
{
	auto &builtinBlocks = BuiltinBlockTypeManager::GetInstance();
	BlockID stoneID     = builtinBlocks.GetDesc(BuiltinBlockType::Stone).desc->GetBlockID();
	BlockID soilID      = builtinBlocks.GetDesc(BuiltinBlockType::Soil).desc ->GetBlockID();
	BlockID lawnID      = builtinBlocks.GetDesc(BuiltinBlockType::Lawn).desc ->GetBlockID();
    BlockID glowStoneID = builtinBlocks.GetDesc(BuiltinBlockType::GlowStone).desc->GetBlockID();
	
    for(int x = 0; x < CHUNK_SIZE_X; ++x)
    {
        for(int z = 0; z < CHUNK_SIZE_Z; ++z)
        {
            int height = (x + z) / 3;

            for(int y = 0; y < height - 3; ++y)
                blockData->SetID({ x, y, z }, stoneID, {});
			for(int y = (std::max)(height - 3, 0); y < height; ++y)
                blockData->SetID({ x, y ,z }, soilID, {});
            blockData->SetID({ x, height, z }, lawnID, {});

            //for(int y = 0; y <= height; ++y)
            //    blockData->SetID({ x, y, z }, 1, {});

            blockData->SetHeight(x, z, height);
        }
    }
}

VRPG_WORLD_END
