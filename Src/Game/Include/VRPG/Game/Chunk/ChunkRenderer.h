#pragma once

#include <VRPG/Game/Block/BlockEffect.h>

VRPG_GAME_BEGIN

class ChunkRenderer : public agz::misc::uncopyable_t
{
public:

    ChunkRenderer();

    void AddPartialSectionModel(std::shared_ptr<const PartialSectionModel> model);

    void Done();

    void RenderForward(const BlockForwardRenderParams &params) const;

    void RenderShadow(const BlockShadowRenderParams &params) const;

    void Clear();

private:

    using ChunkModelSet = std::vector<std::shared_ptr<const PartialSectionModel>>;

    std::vector<ChunkModelSet> modelSets_;
    mutable ChunkModelSet transparentModelSet_;
};

VRPG_GAME_END
