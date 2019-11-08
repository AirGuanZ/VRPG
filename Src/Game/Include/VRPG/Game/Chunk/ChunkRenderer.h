#pragma once

#include <VRPG/Game/Block/BlockEffect.h>

VRPG_GAME_BEGIN

class ChunkRenderer : public agz::misc::uncopyable_t
{
    using ChunkModelSet = std::vector<std::shared_ptr<const PartialSectionModel>>;

    std::vector<ChunkModelSet> modelSets_;
    mutable ChunkModelSet transparentModelSet_;

public:

    ChunkRenderer()
    {
        size_t blockEffectCount = BlockEffectManager::GetInstance().GetBlockEffectCount();
        modelSets_.resize(blockEffectCount);
    }

    void AddPartialSectionModel(std::shared_ptr<const PartialSectionModel> model)
    {
        const BlockEffect *effect = model->GetBlockEffect();
        BlockEffectID effectID = effect->GetBlockEffectID();
        modelSets_[effectID].push_back(std::move(model));
    }

    void Done()
    {
        std::vector<ChunkModelSet> newSolidModelSets;
        for(auto &modelSet : modelSets_)
        {
            if(modelSet.empty())
                continue;

            if(!modelSet.front()->GetBlockEffect()->IsTransparent())
                newSolidModelSets.push_back(std::move(modelSet));
            else
                transparentModelSet_.swap(modelSet);
        }
        modelSets_.swap(newSolidModelSets);
    }

    void Render(const BlockRenderParams &params) const
    {
        for(auto &chunkModelSet : modelSets_)
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

        if(!transparentModelSet_.empty())
        {
            Vec3 cameraPosition = params.camera->GetPosition();
            std::sort(transparentModelSet_.begin(), transparentModelSet_.end(),
                [eye = cameraPosition](auto &lhs, auto &rhs)
            {
                Vec3i lSec = lhs->GetGlobalSectionPosition();
                Vec3i rSec = rhs->GetGlobalSectionPosition();
                Vec3 lCen = {
                    float(CHUNK_SECTION_SIZE_X * lSec.x + CHUNK_SECTION_SIZE_X / 2),
                    float(CHUNK_SECTION_SIZE_Y * lSec.y + CHUNK_SECTION_SIZE_Y / 2),
                    float(CHUNK_SECTION_SIZE_Z * lSec.z + CHUNK_SECTION_SIZE_Z / 2)
                };
                Vec3 rCen = {
                    float(CHUNK_SECTION_SIZE_X * rSec.x + CHUNK_SECTION_SIZE_X / 2),
                    float(CHUNK_SECTION_SIZE_Y * rSec.y + CHUNK_SECTION_SIZE_Y / 2),
                    float(CHUNK_SECTION_SIZE_Z * rSec.z + CHUNK_SECTION_SIZE_Z / 2)
                };
                return (lCen - eye).length_square() > (rCen - eye).length_square();
            });

            auto effect = transparentModelSet_.front()->GetBlockEffect();
            effect->SetRenderParams(params);
            effect->Bind();
            for(auto &model : transparentModelSet_)
            {
                model->Render(*params.camera);
            }
            effect->Unbind();
        }
    }

    void Clear()
    {
        modelSets_.clear();
        transparentModelSet_.clear();

        size_t blockEffectCount = BlockEffectManager::GetInstance().GetBlockEffectCount();
        modelSets_.resize(blockEffectCount);
    }
};

VRPG_GAME_END
