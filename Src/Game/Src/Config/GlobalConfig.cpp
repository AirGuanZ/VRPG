#include <VRPG/Game/Config/GlobalConfig.h>

VRPG_GAME_BEGIN

GlobalConfig GLOBAL_CONFIG;

void ShadowMapConfig::Load(const libconfig::Setting &setting)
{
    setting.lookupValue("DepthBias", shadowDepthBias);
    setting.lookupValue("DepthBiasClamp", shadowDepthBiasClamp);
    setting.lookupValue("DepthSlope", shadowDepthSlope);

    setting.lookupValue("Distance", shadowMapDistance);
    setting.lookupValue("Radius", shadowMapRadius);
    setting.lookupValue("Near", shadowMapNear);
    setting.lookupValue("Far", shadowMapFar);
}

void ShadowMapConfig::Print()
{
    spdlog::info("ShadowMap::DepthBias      = {}", shadowDepthBias);
    spdlog::info("ShadowMap::DepthBiasClamp = {}", shadowDepthBiasClamp);
    spdlog::info("ShadowMap::DepthSlope     = {}", shadowDepthSlope);
    spdlog::info("ShadowMap::Distance       = {}", shadowMapDistance);
    spdlog::info("ShadowMap::Radius         = {}", shadowMapRadius);
    spdlog::info("ShadowMap::Near           = {}", shadowMapNear);
    spdlog::info("ShadowMap::Far            = {}", shadowMapFar);
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

GlobalConfig::GlobalConfig()
    : SHADOW_MAP(shadowMap_), CHUNK_MANAGER(chunkManager_), WINDOW(window_)
{
    
}

void GlobalConfig::LoadFromFile(const char *configFilename)
{
    libconfig::Config config;
    config.readFile(configFilename);

    if(config.exists("ShadowMap"))
        shadowMap_.Load(config.lookup("ShadowMap"));

    if(config.exists("ChunkManager"))
        chunkManager_.Load(config.lookup("ChunkManager"));

    if(config.exists("Window"))
        window_.Load(config.lookup("Window"));

    shadowMap_   .Print();
    chunkManager_.Print();
    window_      .Print();
}

VRPG_GAME_END
