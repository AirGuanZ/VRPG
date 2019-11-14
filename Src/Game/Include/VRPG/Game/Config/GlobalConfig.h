#pragma once

#include <VRPG/Game/Common.h>

VRPG_GAME_BEGIN

class GlobalGraphicsConfig : public Base::Singleton<GlobalGraphicsConfig>
{
    UINT  shadowDepthBias_      = 0;
    float shadowDepthBiasClamp_ = 0;
    float shadowDepthSlope_     = 0;

public:

    void LoadFromFile(const char *configFilename);

    UINT GetShadowDepthBias()       const noexcept { return shadowDepthBias_;      }
    float GetShadowDepthBiasClamp() const noexcept { return shadowDepthBiasClamp_; }
    float GetShadowDepthSlope()     const noexcept { return shadowDepthSlope_;     }
};

VRPG_GAME_END
