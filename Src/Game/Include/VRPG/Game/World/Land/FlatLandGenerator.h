#pragma once

#include <VRPG/Game/World/Land/LandGenerator.h>

VRPG_GAME_BEGIN

class FlatLandGenerator : public LandGenerator
{
    int landHeight_;

public:

    explicit FlatLandGenerator(int landHeight) noexcept;

    void Generate(const ChunkPosition &position, ChunkBlockData *blockData) override;
};

VRPG_GAME_END
