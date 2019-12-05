#pragma once

#include <libconfig.h++>

#include <VRPG/Game/Common.h>

VRPG_GAME_BEGIN

class AssetPathManager
{
    libconfig::Config config_;

public:

    const libconfig::Setting &operator[](const char *name) const { return config_.getRoot()[name]; }

    void LoadFromFile(const char *filename) { Clear(); config_.readFile(filename); }

    void Clear() { config_.clear(); }
};

VRPG_GAME_END
