#pragma once

#include <agz/utility/misc.h>

#include <VRPG/World/Block/BlockEffect.h>
#include <VRPG/World/Chunk/Chunk.h>

VRPG_WORLD_BEGIN

class ChunkRenderer : public agz::misc::uncopyable_t
{
    using ChunkModelSet = std::vector<std::shared_ptr<const PartialSectionModel>>;

    std::vector<ChunkModelSet> chunkModelSets_;

public:

    ChunkRenderer()
    {
        size_t blockEffectCount = BlockEffectManager::GetInstance().GetBlockEffectCount();
        chunkModelSets_.resize(blockEffectCount);
    }

    void AddPartialSectionModel(std::shared_ptr<const PartialSectionModel> model)
    {
        const BlockEffect *effect = model->GetBlockEffect();
        BlockEffectID effectID = effect->GetBlockEffectID();
        assert(effectID < BlockEffectID(chunkModelSets_.size()));
        chunkModelSets_[effectID].push_back(std::move(model));
    }

    void Render(const BlockRenderParams &params) const
    {
        for(auto &chunkModelSet : chunkModelSets_)
        {
            if(chunkModelSet.empty())
                continue;

            auto effect = chunkModelSet.front()->GetBlockEffect();
            effect->SetRenderParams(params);
            effect->Bind();
            for(auto &chunkModel : chunkModelSet)
                chunkModel->Render(*params.camera);
            effect->Unbind();
        }
    }

    void Clear()
    {
        size_t blockEffectCount = BlockEffectManager::GetInstance().GetBlockEffectCount();
        chunkModelSets_.clear();
        chunkModelSets_.resize(blockEffectCount);
    }
};

VRPG_WORLD_END