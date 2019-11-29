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

    virtual void SetRenderParams(const ForwardRenderParams &params) = 0;

    virtual void Bind() = 0;

    virtual void Unbind() = 0;
};

/**
 * @brief 用于shadow texture的生成
 */
class CascadeShadowMapping : public agz::misc::uncopyable_t
{
public:

    virtual ~CascadeShadowMapping() = default;

    virtual void UpdateCSMParams(const Camera &camera) = 0;

    virtual void StartNear() = 0;

    virtual void EndNear() = 0;

    virtual void StartMiddle() = 0;

    virtual void EndMiddle() = 0;

    virtual void StartFar() = 0;

    virtual void EndFar() = 0;

    virtual void FillNearShadowParams(ShadowRenderParams &params) const noexcept = 0;

    virtual void FillMiddleShadowParams(ShadowRenderParams &params) const noexcept = 0;

    virtual void FillFarShadowParams(ShadowRenderParams &params) const noexcept = 0;

    virtual void RenderNearChunkShadow(const ChunkRenderer &chunkRenderer) = 0;

    virtual void RenderMiddleChunkShadow(const ChunkRenderer &chunkRenderer) = 0;

    virtual void RenderFarChunkShadow(const ChunkRenderer &chunkRenderer) = 0;

    virtual void FillForwardParams(ForwardRenderParams &params) = 0;
};

VRPG_GAME_END
