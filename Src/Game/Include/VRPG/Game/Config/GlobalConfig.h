#pragma once

#include <libconfig.h++>

#include <VRPG/Game/Common.h>
#include <VRPG/Game/Config/AssetPathManager.h>

VRPG_GAME_BEGIN

struct ShadowMapConfig
{
    bool enable = true;

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

struct PlayerConfig
{
    float runningAccel          = 300.0f;   // 跑动自给水平加速度
    float walkingAccel          = 200.0f;   // 行走自给水平加速度
    float floatingAccel         = 200.0f;   // 浮空自给水平加速度

    float runningMaxSpeed       = 8.5f;     // 跑动最大自给水平速度
    float walkingMaxSpeed       = 6.5f;     // 行走最大自给水平速度
    float floatingMaxSpeed      = 6.5f;     // 跳跃最大自给水平速度

    float standingFrictionAccel = 80.0f;    // 站立外来阻力水平加速度
    float runningFrictionAccel  = 80.0f;    // 跑动外来阻力水平加速度
    float walkingFrictionAccel  = 80.0f;    // 行走外来阻力水平加速度
    float floatingFrictionAccel = 10.0f;    // 跳跃外来阻力水平加速度

    float jumpingInitVelocity   = 13.0f;    // 跳跃初始速度
    float gravityAccel          = 30.0f;    // 重力加速度
    float gravityMaxSpeed       = 30000.0f; // 最大下落速度

    float cameraMoveXSpeed      = 0.002f;   // 摄像机水平灵敏度
    float cameraMoveYSpeed      = 0.002f;   // 摄像机垂直灵敏度
    
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

struct MiscConfig
{
    bool enableChoseBlockWireframe = false;

    void Load(const libconfig::Setting &setting);

    void Print();
};

class GlobalConfig
{
public:

    GlobalConfig();

    void LoadFromFile(const char *configFilename);

    const ChunkManagerConfig &CHUNK_MANAGER;
    const MiscConfig         &MISC;
    const PlayerConfig       &PLAYER;
    const ShadowMapConfig    &SHADOW_MAP;
    const WindowConfig       &WINDOW;

    AssetPathManager ASSET_PATH;

private:

    ChunkManagerConfig chunkManager_;
    MiscConfig         misc_;
    PlayerConfig       player_;
    ShadowMapConfig    shadowMap_;
    WindowConfig       window_;
};

extern GlobalConfig GLOBAL_CONFIG;

VRPG_GAME_END
