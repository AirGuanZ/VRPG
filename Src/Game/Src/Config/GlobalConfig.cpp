#include <libconfig.h++>

#include <VRPG/Game/Config/GlobalConfig.h>

VRPG_GAME_BEGIN

void GlobalGraphicsConfig::LoadFromFile(const char *configFilename)
{
    libconfig::Config config;
    config.readFile(configFilename);

    if(config.exists("ShadowMap"))
    {
        auto &settings = config.lookup("ShadowMap");
        settings.lookupValue("DepthBias",      shadowDepthBias_);
        settings.lookupValue("DepthBiasClamp", shadowDepthBiasClamp_);
        settings.lookupValue("DepthSlope",     shadowDepthSlope_);
    }

    spdlog::info("GlobalGraphics::ShadowMap::DepthBias      = {}", shadowDepthBias_);
    spdlog::info("GlobalGraphics::ShadowMap::DepthBiasClamp = {}", shadowDepthBiasClamp_);
    spdlog::info("GlobalGraphics::ShadowMap::DepthSlope     = {}", shadowDepthSlope_);
}

VRPG_GAME_END
