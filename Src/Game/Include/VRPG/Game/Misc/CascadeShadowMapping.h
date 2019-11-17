#pragma once

#include <VRPG/Game/Camera/Camera.h>
#include <VRPG/Game/Chunk/ChunkRenderer.h>

VRPG_GAME_BEGIN

class CascadeShadowMapping : public agz::misc::uncopyable_t
{
public:

    CascadeShadowMapping();

    void RenderShadow(const Camera &camera, const ChunkRenderer &chunkRenderer);

    void FillForwardParams(BlockForwardRenderParams &params);

private:

    static void UpdateViewProj(const Camera &camera, Mat4 viewProj[3], float cascadeZLimit[3]);

    Mat4 viewProj_[3];
    float cascadeZLimit_[3];

    std::unique_ptr<Base::ShadowMap> nearSM_;
    std::unique_ptr<Base::ShadowMap> middleSM_;
    std::unique_ptr<Base::ShadowMap> farSM_;
};

VRPG_GAME_END
