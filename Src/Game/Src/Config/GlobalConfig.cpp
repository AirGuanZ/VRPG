#include <agz/utility/string.h>

#include <VRPG/Game/Config/GlobalConfig.h>

VRPG_GAME_BEGIN

namespace
{
    template<typename T>
    void PrintItem(const char *str, const T &value)
    {
        std::string fmtStr = agz::stdstr::align_left(str, 36, ' ') + "= {}";
        spdlog::info(fmtStr, value);
    }
}

GlobalConfig GLOBAL_CONFIG;

void ShadowMapConfig::Load(const libconfig::Setting &setting)
{
    setting.lookupValue("Enable", enable);

    setting.lookupValue("Resolution0", resolution[0]);
    setting.lookupValue("Resolution1", resolution[1]);
    setting.lookupValue("Resolution2", resolution[2]);

    setting.lookupValue("DepthBias",      depthBias);
    setting.lookupValue("DepthBiasClamp", depthBiasClamp);
    setting.lookupValue("DepthSlope",     depthSlope);

    setting.lookupValue("Distance", distance);
}

void ShadowMapConfig::Print()
{
    PrintItem("ShadowMap::Enable",         enable);
    PrintItem("ShadowMap::Resolution[0]",  resolution[0]);
    PrintItem("ShadowMap::Resolution[1]",  resolution[1]);
    PrintItem("ShadowMap::Resolution[2]",  resolution[2]);
    PrintItem("ShadowMap::DepthBias",      depthBias);
    PrintItem("ShadowMap::DepthBiasClamp", depthBiasClamp);
    PrintItem("ShadowMap::DepthSlope",     depthSlope);
    PrintItem("ShadowMap::Distance",       distance);
}

void ChunkManagerConfig::Load(const libconfig::Setting &setting)
{
    setting.lookupValue("RenderDistance", renderDistance);
    setting.lookupValue("LoadDistance",   loadDistance);
    setting.lookupValue("UnloadDistance", unloadDistance);

    setting.lookupValue("BackgroundPoolSize",    backgroundPoolSize);
    setting.lookupValue("BackgroundThreadCount", backgroundThreadCount);
}

void ChunkManagerConfig::Print()
{
    PrintItem("ChunkManager::RenderDistance",        renderDistance);
    PrintItem("ChunkManager::LoadDistance",          loadDistance);
    PrintItem("ChunkManager::UnloadDistance",        unloadDistance);
    PrintItem("ChunkManager::BackgroundPoolSize",    backgroundPoolSize);
    PrintItem("ChunkManager::BackgroundThreadCount", backgroundThreadCount);
}

void PlayerConfig::Load(const libconfig::Setting &setting)
{
    setting.lookupValue("RunningAccel",  runningAccel);
    setting.lookupValue("WalkingAccel",  walkingAccel);
    setting.lookupValue("FloatingAccel", floatingAccel);

    setting.lookupValue("RunningMaxSpeed",  runningMaxSpeed);
    setting.lookupValue("WalkingMaxSpeed",  walkingMaxSpeed);
    setting.lookupValue("FloatingMaxSpeed", floatingMaxSpeed);

    setting.lookupValue("StandingFricAccel", standingFrictionAccel);
    setting.lookupValue("RunningFricAccel",  runningFrictionAccel);
    setting.lookupValue("WalkingFricAccel",  walkingFrictionAccel);
    setting.lookupValue("FloatingFricAccel", floatingFrictionAccel);

    setting.lookupValue("JumpingInitVelocity", jumpingInitVelocity);
    setting.lookupValue("GravityAccel",        gravityAccel);
    setting.lookupValue("GravityMaxSpeed",  gravityMaxSpeed);

    setting.lookupValue("FlyingAccel",         flyingAccel);
    setting.lookupValue("FlyingFricAccel",     flyingFricAccel);
    setting.lookupValue("FlyingMaxSpeed",      flyingMaxSpeed);
    setting.lookupValue("FastFlyingMaxSpeed",  fastFlyingMaxSpeed);
    setting.lookupValue("FlyingVertAccel",     flyingVertAccel);
    setting.lookupValue("FlyingVertFricAccel", flyingVertFricAccel);
    setting.lookupValue("FlyingVertMaxSpeed",  flyingVertMaxSpeed);

    setting.lookupValue("CameraMoveXSpeed", cameraMoveXSpeed);
    setting.lookupValue("CameraMoveYSpeed", cameraMoveYSpeed);
}

void PlayerConfig::Print()
{
    PrintItem("Player::RunningAccel",        runningAccel);
    PrintItem("Player::WalkingAccel",        walkingAccel);
    PrintItem("Player::FloatingAccel",       floatingAccel);
    PrintItem("Player::RunningMaxSpeed",     runningMaxSpeed);
    PrintItem("Player::WalkingMaxSpeed",     walkingMaxSpeed);
    PrintItem("Player::FloatingMaxSpeed",    floatingMaxSpeed);
    PrintItem("Player::StandingFricAccel",   standingFrictionAccel);
    PrintItem("Player::WalkingFricAccel",    walkingFrictionAccel);
    PrintItem("Player::RunningFricAccel",    runningFrictionAccel);
    PrintItem("Player::FloatingFricAccel",   floatingFrictionAccel);
    PrintItem("Player::JumpingInitVelocity", jumpingInitVelocity);
    PrintItem("Player::GravityAccel",        gravityAccel);
    PrintItem("Player::GravityMaxSpeed",     gravityMaxSpeed);
    PrintItem("FlyingAccel",                 flyingAccel);
    PrintItem("FlyingFricAccel",             flyingFricAccel);
    PrintItem("FlyingMaxSpeed",              flyingMaxSpeed);
    PrintItem("FastFlyingMaxSpeed",          fastFlyingMaxSpeed);
    PrintItem("FlyingVertAccel",             flyingVertAccel);
    PrintItem("FlyingVertFricAccel",         flyingVertFricAccel);
    PrintItem("FlyingVertMaxSpeed",          flyingVertMaxSpeed);
    PrintItem("Player::CameraMoveXSpeed",    cameraMoveXSpeed);
    PrintItem("Player::CameraMoveYSpeed",    cameraMoveYSpeed);
}

void WindowConfig::Load(const libconfig::Setting &setting)
{
    setting.lookupValue("Width",      width);
    setting.lookupValue("Height",     height);
    setting.lookupValue("Fullscreen", fullscreen);
    setting.lookupValue("VSync",      vsync);

    setting.lookupValue("MSAACount",   sampleCount);
    setting.lookupValue("MSAAQuality", sampleQuality);
}

void WindowConfig::Print()
{
    PrintItem("Window::Width",       width);
    PrintItem("Window::Height",      height);
    PrintItem("Window::Fullscreen",  fullscreen);
    PrintItem("Window::VSync",       vsync);
    PrintItem("Window::MSAACount",   sampleCount);
    PrintItem("Window::MSAAQuality", sampleQuality);
}

void MiscConfig::Load(const libconfig::Setting &setting)
{
    setting.lookupValue("EnableChosenBlockWireframe", enableChoseBlockWireframe);
}

void MiscConfig::Print()
{
    PrintItem("Misc::EnableChosenBlockWireframe", enableChoseBlockWireframe);
}

GlobalConfig::GlobalConfig()
    : CHUNK_MANAGER(chunkManager_), MISC(misc_), PLAYER(player_), SHADOW_MAP(shadowMap_), WINDOW(window_)
{
    
}

void GlobalConfig::LoadFromFile(const char *configFilename)
{
    libconfig::Config config;
    config.readFile(configFilename);

    if(config.exists("ChunkManager"))
    {
        chunkManager_.Load(config.lookup("ChunkManager"));
    }

    if(config.exists("Misc"))
    {
        misc_.Load(config.lookup("Misc"));
    }

    if(config.exists("Player"))
    {
        player_.Load(config.lookup("Player"));
    }

    if(config.exists("ShadowMap"))
    {
        shadowMap_.Load(config.lookup("ShadowMap"));
    }

    if(config.exists("Window"))
    {
        window_.Load(config.lookup("Window"));
    }

    chunkManager_.Print();
    misc_        .Print();
    player_      .Print();
    shadowMap_   .Print();
    window_      .Print();
}

VRPG_GAME_END
