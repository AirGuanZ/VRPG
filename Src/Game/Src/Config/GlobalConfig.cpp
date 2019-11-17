#include <VRPG/Game/Config/GlobalConfig.h>

VRPG_GAME_BEGIN

GlobalConfig GLOBAL_CONFIG;

void ShadowMapConfig::Load(const libconfig::Setting &setting)
{
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
    spdlog::info("ShadowMap::Resolution[0] = {}", resolution[0]);
    spdlog::info("ShadowMap::Resolution[1] = {}", resolution[1]);
    spdlog::info("ShadowMap::Resolution[2] = {}", resolution[2]);

    spdlog::info("ShadowMap::DepthBias      = {}", depthBias);
    spdlog::info("ShadowMap::DepthBiasClamp = {}", depthBiasClamp);
    spdlog::info("ShadowMap::DepthSlope     = {}", depthSlope);

    spdlog::info("ShadowMap::Distance       = {}", distance);
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
    spdlog::info("ChunkManager::RenderDistance        = {}", renderDistance);
    spdlog::info("ChunkManager::LoadDistance          = {}", loadDistance);
    spdlog::info("ChunkManager::UnloadDistance        = {}", unloadDistance);
    spdlog::info("ChunkManager::BackgroundPoolSize    = {}", backgroundPoolSize);
    spdlog::info("ChunkManager::BackgroundThreadCount = {}", backgroundThreadCount);
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
    spdlog::info("Window::Width       = {}", width);
    spdlog::info("Window::Height      = {}", height);
    spdlog::info("Window::Fullscreen  = {}", fullscreen);
    spdlog::info("Window::VSync       = {}", vsync);
    spdlog::info("Window::MSAACount   = {}", sampleCount);
    spdlog::info("Window::MSAAQuality = {}", sampleQuality);
}

void MiscConfig::Load(const libconfig::Setting &setting)
{
    setting.lookupValue("EnableChosenBlockWireframe", enableChoseBlockWireframe);
}

void MiscConfig::Print()
{
    spdlog::info("Misc::EnableChosenBlockWireframe = {}", enableChoseBlockWireframe);
}

GlobalConfig::GlobalConfig()
    : CHUNK_MANAGER(chunkManager_), MISC(misc_), SHADOW_MAP(shadowMap_), WINDOW(window_)
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
    shadowMap_   .Print();
    window_      .Print();
}

VRPG_GAME_END
