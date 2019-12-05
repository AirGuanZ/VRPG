#pragma once

#include <VRPG/Game/Misc/RenderParams.h>

VRPG_GAME_BEGIN

/**
 * @brief 通用mesh rendering effect，统一要求实现forward和shadow两个pass
 */
class MeshEffect : public agz::misc::uncopyable_t
{
public:

    virtual ~MeshEffect() = default;

    // forward rendering

    virtual void SetForwardRenderParams(const ForwardRenderParams &params) = 0;

    virtual void StartForward() const = 0;

    virtual void EndForward() const = 0;

    // shadow rendering

    virtual void SetShadowRenderParams(const ShadowRenderParams &params) = 0;

    virtual void StartShadow() const = 0;

    virtual void EndShadow() const = 0;
};

VRPG_GAME_END
