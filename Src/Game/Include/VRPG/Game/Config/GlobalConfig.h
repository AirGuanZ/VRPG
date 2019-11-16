#pragma once

#include <libconfig.h++>

#include <VRPG/Game/Common.h>
#include <VRPG/Game/Config/AssetPathManager.h>

VRPG_GAME_BEGIN

struct ShadowMapConfig
{
    int resolution[3] = { 4096, 4096, 4096 };

    UINT  depthBias      = 0;
    float depthBiasClamp = 0;
    float depthSlope     = 0;

    float distance = 60;

    void Load(const libconfig::Setting &setting);

    void Print();
};

struct ChunkManagerConfig
{
    int renderDistance = 2;
    int loadDistance   = 3;
    int unloadDistance = 5;

    int backgroundPoolSize    = 20;
    int backgroundThreadCount = 1;

    void Load(const libconfig::Setting &setting);

    void Print();
};

struct WindowConfig
{
    int width       = 640;
    int height      = 480;
    bool fullscreen = false;
    bool vsync      = true;

    int sampleCount   = 4;
    int sampleQuality = 1;

    void Load(const libconfig::Setting &setting);

    void Print();
};

class GlobalConfig
{
public:

    GlobalConfig();

    void LoadFromFile(const char *configFilename);

    const ShadowMapConfig    &SHADOW_MAP;
    const ChunkManagerConfig &CHUNK_MANAGER;
    const WindowConfig       &WINDOW;

    AssetPathManager ASSET_PATH;

private:

    ShadowMapConfig    shadowMap_;
    ChunkManagerConfig chunkManager_;
    WindowConfig       window_;
};

extern GlobalConfig GLOBAL_CONFIG;

VRPG_GAME_END
