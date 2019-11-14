#pragma once

#include <VRPG/Game/Common.h>

VRPG_GAME_BEGIN

class GlobalGraphicsConfig : public Base::Singleton<GlobalGraphicsConfig>
{
public:

    void LoadFromFile(const char *configFilename);

    struct ShadowMapConfig
    {
        UINT  shadowDepthBias      = 0;
        float shadowDepthBiasClamp = 0;
        float shadowDepthSlope     = 0;

        float shadowMapDistance = 60;
        float shadowMapRadius   = 40;
        float shadowMapNear     = 50;
        float shadowMapFar      = 200;
    };

    const ShadowMapConfig &GetShadowMapConfig() const noexcept { return shadowMap_; }

private:

    ShadowMapConfig shadowMap_;
};

VRPG_GAME_END
