#include <VRPG/Game/Block/BlockDescription.h>
#include <VRPG/Game/Block/BuiltinBlock/BuiltinBlock.h>
#include <VRPG/Game/Land/FlatLandGenerator.h>

VRPG_GAME_BEGIN

FlatLandGenerator::FlatLandGenerator(int landHeight) noexcept
    : landHeight_(landHeight)
{
    
}

void FlatLandGenerator::Generate(const ChunkPosition &position, ChunkBlockData *blockData)
{
    auto &builtinBlocks = BuiltinBlockTypeManager::GetInstance();
    BlockID stoneID      = builtinBlocks.GetDesc(BuiltinBlockType::Stone)     .desc->GetBlockID();
    BlockID soilID       = builtinBlocks.GetDesc(BuiltinBlockType::Soil)      .desc->GetBlockID();
    BlockID lawnID       = builtinBlocks.GetDesc(BuiltinBlockType::Lawn)      .desc->GetBlockID();
    BlockID glowStoneID  = builtinBlocks.GetDesc(BuiltinBlockType::GlowStone) .desc->GetBlockID();
    BlockID leafID       = builtinBlocks.GetDesc(BuiltinBlockType::Leaf)      .desc->GetBlockID();
    BlockID whiteGlassID = builtinBlocks.GetDesc(BuiltinBlockType::WhiteGlass).desc->GetBlockID();
    BlockID redGlassID   = builtinBlocks.GetDesc(BuiltinBlockType::RedGlass)  .desc->GetBlockID();
    BlockID logID        = builtinBlocks.GetDesc(BuiltinBlockType::Log)       .desc->GetBlockID();

    for(int x = 0; x < CHUNK_SIZE_X; ++x)
    {
        for(int z = 0; z < CHUNK_SIZE_Z; ++z)
        {
            int height = landHeight_;

            for(int y = 0; y < height - 3; ++y)
                blockData->SetID({ x, y, z }, stoneID, {});
            for(int y = (std::max)(height - 3, 0); y < height; ++y)
                blockData->SetID({ x, y ,z }, soilID, {});
            blockData->SetID({ x, height, z }, lawnID, {});

            if(x == 4 && z == 4)
                blockData->SetID({ x, ++height, z }, glowStoneID, {});

            if(6 <= x && x <= 7 && 8 <= z && z <= 9)
                blockData->SetID({ x, ++height, z }, leafID, {});

            if(12 <= x && x <= 13 && 12 <= z && z <= 13)
                blockData->SetID({ x, ++height, z }, whiteGlassID, {});
            if(12 <= x && x <= 13 && 14 <= z && z <= 15)
                blockData->SetID({ x, ++height, z }, redGlassID, {});

            if(x == 15 && z == 15)
                blockData->SetID({ x, ++height, z }, logID, {});
            if(x == 16 && z == 16)
                blockData->SetID({ x, ++height, z }, logID, { PositiveY, PositiveZ });
            if(x == 17 && z == 17)
                blockData->SetID({ x, ++height, z }, logID, { PositiveZ, PositiveX });

            //for(int y = 0; y <= height; ++y)
            //    blockData->SetID({ x, y, z }, 1, {});

            blockData->SetHeight(x, z, height);
        }
    }
}

VRPG_GAME_END
