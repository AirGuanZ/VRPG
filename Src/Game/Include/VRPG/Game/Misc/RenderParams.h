#pragma once

#include <VRPG/Game/Player/Camera/Camera.h>

VRPG_GAME_BEGIN

/**
 * @brief 单个shadow map的渲染参数
 */
struct ShadowMapParams
{
    Mat4 shadowViewProj;
    float PCFStep = 1.0f / 4096;
    float homZLimit = 1;
    ComPtr<ID3D11ShaderResourceView> shadowMapSRV;
};

/**
 * @brief 通用区块前向渲染参数
 *
 * 前向渲染时均只依赖于此参数
 */
struct ForwardRenderParams
{
    const Camera *camera = nullptr;
    Vec3 skyLight;

    float shadowScale = 1;
    Vec3 sunlightDirection;
    ShadowMapParams cascadeShadowMaps[3];
};

/**
 * @brief 通用区块shadow map渲染参数
 */
struct ShadowRenderParams
{
    Mat4 shadowViewProj;
};

VRPG_GAME_END
