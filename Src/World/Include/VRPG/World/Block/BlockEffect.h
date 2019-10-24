#pragma once

#include <cassert>
#include <memory>
#include <vector>

#include <VRPG/Base/Singleton.h>
#include <VRPG/World/Chunk/ChunkModel.h>

VRPG_WORLD_BEGIN

using BlockEffectID = uint16_t;

constexpr BlockEffectID BLOCK_EFFECT_ID_INVALID = 0;

inline bool IsValidBlockEffectID(BlockEffectID id) noexcept
{
    return id != BLOCK_EFFECT_ID_INVALID;
}

class BlockEffect
{
    BlockEffectID blockEffectID_ = BLOCK_EFFECT_ID_INVALID;

public:

    virtual ~BlockEffect() = default;

    void SetBlockEffectID(BlockEffectID id) noexcept
    {
        blockEffectID_ = id;
    }

    BlockEffectID GetBlockEffectID() const noexcept
    {
        return blockEffectID_;
    }

    virtual void Bind() const = 0;

    virtual void Unbind() const = 0;

    virtual std::unique_ptr<PartialSectionModelBuilder> CreateModelBuilder() const = 0;
};

class BlockEffectManager : public Base::Singleton<BlockEffectManager>
{
    std::vector<std::shared_ptr<BlockEffect>> blockEffects_;
    std::vector<BlockEffect*> rawBlockEffects_;

public:

    BlockEffectID RegisterBlockEffect(std::shared_ptr<BlockEffect> effect)
    {
        assert(blockEffects_.size() < (std::numeric_limits<BlockEffectID>::max)());
        BlockEffectID ret = BlockEffectID(blockEffects_.size());
        effect->SetBlockEffectID(ret);
        rawBlockEffects_.push_back(effect.get());
        blockEffects_.push_back(std::move(effect));
        return ret;
    }

    size_t GetBlockEffectCount() const noexcept
    {
        return blockEffects_.size();
    }

    const BlockEffect *GetBlockEffect(BlockEffectID id) const noexcept
    {
        assert(id < BlockEffectID(rawBlockEffects_.size()));
        return rawBlockEffects_[id];
    }

    void Clear()
    {
        blockEffects_.clear();
        rawBlockEffects_.clear();
    }
};

VRPG_WORLD_END
