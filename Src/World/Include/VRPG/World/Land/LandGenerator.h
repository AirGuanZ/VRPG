#pragma once

#include <VRPG/World/Chunk/Chunk.h>

VRPG_WORLD_BEGIN

class ChunkBlockDataPool;

class LandGenerator
{
public:

    virtual ~LandGenerator() = default;

    /**
     * @brief 输入chunk的位置是有效的，其他字段均无效
     */
    virtual void Generate(const ChunkPosition &position, ChunkBlockData *blockData) = 0;
};

VRPG_WORLD_END
