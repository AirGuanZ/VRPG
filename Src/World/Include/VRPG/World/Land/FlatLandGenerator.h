#pragma once

#include <VRPG/World/Land/LandGenerator.h>

VRPG_WORLD_BEGIN

class FlatLandGenerator : public LandGenerator
{
    int landHeight_;

public:

    explicit FlatLandGenerator(int landHeight) noexcept;

    void Generate(const ChunkPosition &position, ChunkBlockData *blockData) override;
};

VRPG_WORLD_END
