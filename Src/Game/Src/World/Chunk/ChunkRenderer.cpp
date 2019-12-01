#include <VRPG/Game/World/Chunk/ChunkRenderer.h>

VRPG_GAME_BEGIN

namespace
{
    CullingBoundingBox GetSectionBoundingBox(const Vec3i &globalSectionPosition) noexcept
    {
        CullingBoundingBox bbox;
        bbox.low =
        {
            float(globalSectionPosition.x * CHUNK_SECTION_SIZE_X),
            float(globalSectionPosition.y * CHUNK_SECTION_SIZE_Y),
            float(globalSectionPosition.z * CHUNK_SECTION_SIZE_Z)
        };
        bbox.high = {
            bbox.low.x + CHUNK_SECTION_SIZE_X,
            bbox.low.y + CHUNK_SECTION_SIZE_Y,
            bbox.low.z + CHUNK_SECTION_SIZE_Z
        };
        return bbox;
    }

    bool ShouldRender(const Vec3i &globalSectionPosition, const FrustumCuller &culler) noexcept
    {
        return culler.IsVisible(GetSectionBoundingBox(globalSectionPosition));
    }

    bool ShouldRender(const Vec3i &globalSectionPosition, const Camera &camera) noexcept
    {
        return camera.IsVisible(GetSectionBoundingBox(globalSectionPosition));
    }
}

ChunkRenderer::ChunkRenderer()
{
    size_t blockEffectCount = BlockEffectManager::GetInstance().GetBlockEffectCount();
    modelSets_.resize(blockEffectCount);
}

void ChunkRenderer::AddPartialSectionModel(std::shared_ptr<const PartialSectionModel> model)
{
    const BlockEffect *effect = model->GetBlockEffect();
    BlockEffectID effectID = effect->GetBlockEffectID();
    modelSets_[effectID].push_back(std::move(model));
}

void ChunkRenderer::Done()
{
    std::vector<ChunkModelSet> newSolidModelSets;
    for(auto &modelSet : modelSets_)
    {
        if(modelSet.empty())
        {
            continue;
        }

        if(!modelSet.front()->GetBlockEffect()->IsTransparent())
        {
            newSolidModelSets.push_back(std::move(modelSet));
        }
        else
        {
            transparentModelSet_.swap(modelSet);
        }
    }
    modelSets_.swap(newSolidModelSets);
}

void ChunkRenderer::RenderForwardOpaque(const ForwardRenderParams &params) const
{
    for(auto &chunkModelSet : modelSets_)
    {
        if(chunkModelSet.empty())
        {
            continue;
        }

        auto effect = chunkModelSet.front()->GetBlockEffect();
        effect->SetForwardRenderParams(params);
        effect->StartForward();
        for(auto &chunkModel : chunkModelSet)
        {
            if(ShouldRender(chunkModel->GetGlobalSectionPosition(), *params.camera))
            {
                chunkModel->Render(*params.camera);
            }
        }
        effect->EndForward();
    }
}

void ChunkRenderer::RenderForwardTransparent(const ForwardRenderParams &params) const
{
    if(!transparentModelSet_.empty())
    {
        Vec3 cameraPosition = params.camera->GetPosition();
        std::sort(transparentModelSet_.begin(), transparentModelSet_.end(),
            [eye = cameraPosition](auto &lhs, auto &rhs)
        {
            Vec3i lSec = lhs->GetGlobalSectionPosition();
            Vec3i rSec = rhs->GetGlobalSectionPosition();
            Vec3 lCen = {
                CHUNK_SECTION_SIZE_X * lSec.x + 0.5f * CHUNK_SECTION_SIZE_X,
                CHUNK_SECTION_SIZE_Y * lSec.y + 0.5f * CHUNK_SECTION_SIZE_Y,
                CHUNK_SECTION_SIZE_Z * lSec.z + 0.5f * CHUNK_SECTION_SIZE_Z
            };
            Vec3 rCen = {
                CHUNK_SECTION_SIZE_X * rSec.x + 0.5f * CHUNK_SECTION_SIZE_X,
                CHUNK_SECTION_SIZE_Y * rSec.y + 0.5f * CHUNK_SECTION_SIZE_Y,
                CHUNK_SECTION_SIZE_Z * rSec.z + 0.5f * CHUNK_SECTION_SIZE_Z
            };
            return (lCen - eye).length_square() > (rCen - eye).length_square();
        });

        auto effect = transparentModelSet_.front()->GetBlockEffect();
        effect->SetForwardRenderParams(params);
        effect->StartForward();
        for(auto &model : transparentModelSet_)
        {
            if(ShouldRender(model->GetGlobalSectionPosition(), *params.camera))
            {
                model->Render(*params.camera);
            }
        }
        effect->EndForward();
    }
}

void ChunkRenderer::RenderShadow(const ShadowRenderParams &params) const
{
    FrustumCuller culler(params.shadowViewProj);

    for(auto &chunkModelSet : modelSets_)
    {
        if(chunkModelSet.empty())
        {
            continue;
        }

        auto effect = chunkModelSet.front()->GetBlockEffect();
        effect->SetShadowRenderParams(params);
        effect->StartShadow();
        for(auto &chunkModel : chunkModelSet)
        {
            if(ShouldRender(chunkModel->GetGlobalSectionPosition(), culler))
            {
                chunkModel->RenderShadow();
            }
        }
        effect->EndShadow();
    }
}

void ChunkRenderer::Clear()
{
    modelSets_.clear();
    transparentModelSet_.clear();

    size_t blockEffectCount = BlockEffectManager::GetInstance().GetBlockEffectCount();
    modelSets_.resize(blockEffectCount);
}

VRPG_GAME_END
