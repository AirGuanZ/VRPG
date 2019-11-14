#include <VRPG/Game/Config/GlobalConfig.h>
#include <VRPG/Game/Misc/ShadowMappingRasterizerState.h>

VRPG_GAME_BEGIN

RasterizerState CreateRasterizerStateForShadowMapping(bool enableBackfaceCulling)
{
    auto &config = GlobalGraphicsConfig::GetInstance().GetShadowMapConfig();
    UINT depthBias       = config.shadowDepthBias;
    float depthBiasClamp = config.shadowDepthBiasClamp;
    float depthSlope     = config.shadowDepthSlope;

    RasterizerState state;
    state.Initialize(
        D3D11_FILL_SOLID,
        enableBackfaceCulling ? D3D11_CULL_BACK : D3D11_CULL_NONE,
        false, depthBias, depthBiasClamp, depthSlope);
    return state;
}

VRPG_GAME_END
