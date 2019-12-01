#pragma once

#include <VRPG/Game/World/Block/BlockEffect.h>

VRPG_GAME_BEGIN

class ChunkRenderer : public agz::misc::uncopyable_t
{
public:

    ChunkRenderer();

    void AddPartialSectionModel(std::shared_ptr<const PartialSectionModel> model);

    void Done();

    void RenderForwardOpaque(const ForwardRenderParams &params) const;

    void RenderForwardTransparent(const ForwardRenderParams &params) const;

    void RenderShadow(const ShadowRenderParams &params) const;

    void Clear();

private:

    using ChunkModelSet = std::vector<std::shared_ptr<const PartialSectionModel>>;

    std::vector<ChunkModelSet> modelSets_;
    mutable ChunkModelSet transparentModelSet_;
};

VRPG_GAME_END
