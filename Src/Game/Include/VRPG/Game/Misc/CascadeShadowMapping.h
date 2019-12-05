#pragma once

#include <VRPG/Game/Player/Camera/Camera.h>
#include <VRPG/Game/World/Block/BlockEffect.h>

/*
实现shadow mapping的辅助类

阴影有个开关选项，因此这些用于每个功能的辅助类都有两个。
它们继承同一接口类，一个在开了阴影时启用，一个在关闭阴影时启动。
*/

VRPG_GAME_BEGIN

class CascadeShadowMapping;
class ForwardShadowMapping;

/**
 * @brief 创建一个用于渲染shadow texture的rasterizer state，主要是使用depth bias相关的参数
 */
RasterizerState CreateRasterizerStateForShadowMapping(bool enableBackfaceCulling = true);

/**
 * @brief 根据阴影开关的值创建一个ForwardShadowMapping实现的实例
 */
std::unique_ptr<ForwardShadowMapping> CreateForwardshadowMapping(UniformManager<SS_VS, SS_PS> *forwardUniforms);

/**
 * @brief 根据阴影开关的值返回一个hlsl宏，打开阴影时为{ "ENABLE_SHADOW", "1" }，关闭时为{ "ENABLE_SHADOW", "0" }
 */
D3D_SHADER_MACRO GetShadowMappingEnableMacro();

/**
 * @brief 根据阴影开关选项创建一个CascadeShadowMapping实现的实例
 */
std::unique_ptr<CascadeShadowMapping> CreateCascadeShadowMapping();

/**
 * @brief 用于设置渲染时的阴影参数
 */
class ForwardShadowMapping
{
public:

    virtual ~ForwardShadowMapping() = default;

    /**
     * @brief 设置前向渲染参数
     */
    virtual void SetRenderParams(const ForwardRenderParams &params) = 0;

    /**
     * @brief 开始前向渲染
     */
    virtual void StartForward() = 0;

    /**
     * @brief 结束前向渲染
     */
    virtual void EndForward() = 0;
};

/**
 * @brief 用于shadow texture的生成
 */
class CascadeShadowMapping : public agz::misc::uncopyable_t
{
public:

    virtual ~CascadeShadowMapping() = default;

    /**
     * @brief 更新阴影矩阵等参数
     */
    virtual void UpdateCSMParams(const Camera &camera) = 0;

    /**
     * @brief 开始渲染到near SM
     */
    virtual void StartNear() = 0;

    /**
     * @brief 结束对near SM的渲染
     */
    virtual void EndNear() = 0;

    /**
     * @brief 开始渲染到middle SM
     */
    virtual void StartMiddle() = 0;

    /**
     *@brief 结束对middle SM的渲染
     */
    virtual void EndMiddle() = 0;

    /**
     * @brief 开始渲染到far SM
     */
    virtual void StartFar() = 0;

    /**
     * @brief 结束对far SM的渲染
     */
    virtual void EndFar() = 0;

    /**
     * @brief 填充针对near SM的渲染参数
     */
    virtual void FillNearShadowParams(ShadowRenderParams &params) const noexcept = 0;

    /**
     * @brief 填充针对middle SM的渲染参数
     */
    virtual void FillMiddleShadowParams(ShadowRenderParams &params) const noexcept = 0;

    /**
     * @brief 填充针对far SM的渲染参数
     */
    virtual void FillFarShadowParams(ShadowRenderParams &params) const noexcept = 0;

    /**
     * @brief 填充用于前向渲染的参数
     */
    virtual void FillForwardParams(ForwardRenderParams &params) = 0;
};

VRPG_GAME_END
