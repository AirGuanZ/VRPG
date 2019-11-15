#include <VRPG/Game/Config/GlobalConfig.h>
#include <VRPG/Game/Misc/ShadowMappingRasterizerState.h>

VRPG_GAME_BEGIN

RasterizerState CreateRasterizerStateForShadowMapping(bool enableBackfaceCulling)
{
    UINT depthBias       = GLOBAL_CONFIG.SHADOW_MAP.shadowDepthBias;
    float depthBiasClamp = GLOBAL_CONFIG.SHADOW_MAP.shadowDepthBiasClamp;
    float depthSlope     = GLOBAL_CONFIG.SHADOW_MAP.shadowDepthSlope;

    RasterizerState state;
    state.Initialize(
        D3D11_FILL_SOLID,
        enableBackfaceCulling ? D3D11_CULL_BACK : D3D11_CULL_NONE,
        false, depthBias, depthBiasClamp, depthSlope);
    return state;
}

VRPG_GAME_END
