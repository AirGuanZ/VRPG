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
