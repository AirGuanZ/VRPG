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

        settings.lookupValue("DepthBias",      shadowMap_.shadowDepthBias);
        settings.lookupValue("DepthBiasClamp", shadowMap_.shadowDepthBiasClamp);
        settings.lookupValue("DepthSlope",     shadowMap_.shadowDepthSlope);

        settings.lookupValue("Distance", shadowMap_.shadowMapDistance);
        settings.lookupValue("Radius",   shadowMap_.shadowMapRadius);
        settings.lookupValue("Near",     shadowMap_.shadowMapNear);
        settings.lookupValue("Far",      shadowMap_.shadowMapFar);
    }

    spdlog::info("GlobalGraphics::ShadowMap::DepthBias      = {}", shadowMap_.shadowDepthBias);
    spdlog::info("GlobalGraphics::ShadowMap::DepthBiasClamp = {}", shadowMap_.shadowDepthBiasClamp);
    spdlog::info("GlobalGraphics::ShadowMap::DepthSlope     = {}", shadowMap_.shadowDepthSlope);
    spdlog::info("GlobalGraphics::ShadowMap::Distance       = {}", shadowMap_.shadowMapDistance);
    spdlog::info("GlobalGraphics::ShadowMap::Radius         = {}", shadowMap_.shadowMapRadius);
    spdlog::info("GlobalGraphics::ShadowMap::Near           = {}", shadowMap_.shadowMapNear);
    spdlog::info("GlobalGraphics::ShadowMap::Far            = {}", shadowMap_.shadowMapFar);
}

VRPG_GAME_END
