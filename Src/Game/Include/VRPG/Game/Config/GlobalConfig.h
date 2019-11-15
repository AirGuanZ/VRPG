#pragma once

#include <libconfig.h++>

#include <VRPG/Game/Common.h>
#include <VRPG/Game/Config/AssetPathManager.h>

VRPG_GAME_BEGIN

struct ShadowMapConfig
{
    UINT  shadowDepthBias      = 0;
    float shadowDepthBiasClamp = 0;
    float shadowDepthSlope     = 0;

    float shadowMapDistance = 60;
    float shadowMapRadius   = 40;
    float shadowMapNear     = 50;
    float shadowMapFar      = 200;

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
