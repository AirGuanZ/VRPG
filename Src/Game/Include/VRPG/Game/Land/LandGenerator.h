#pragma once

#include <VRPG/Game/Chunk/Chunk.h>

VRPG_GAME_BEGIN

class ChunkBlockDataPool;

class LandGenerator
{
protected:

    static void ComputeHeightMap(ChunkBlockData *blockData) noexcept;

public:

    virtual ~LandGenerator() = default;

    virtual void Generate(const ChunkPosition &position, ChunkBlockData *blockData) = 0;
};

VRPG_GAME_END
