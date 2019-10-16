#pragma once

#include <cassert>
#include <memory>
#include <vector>

#include <VRPG/Base/Singleton.h>
#include <VRPG/World/Common.h>

VRPG_WORLD_BEGIN

class ChunkModelBuilder;

enum class Direction : uint8_t
{
    PositiveX = (1 << 1),
    PositiveY = (1 << 2),
    PositiveZ = (1 << 3),
    NegativeX = (1 << 4),
    NegativeY = (1 << 5),
    NegativeZ = (1 << 6)
};

using BlockID = uint16_t;

constexpr BlockID BLOCK_ID_INVALID = 0;

inline bool IsValieBlockID(BlockID id) noexcept
{
    return id != BLOCK_ID_INVALID;
}

class BlockDescription
{
    BlockID blockID_ = BLOCK_ID_INVALID;

public:

    virtual ~BlockDescription() = default;

    void SetBlockID(BlockID id) noexcept
    {
        blockID_ = id;
    }

    BlockID GetBlockID() const noexcept
    {
        return blockID_;
    }

    virtual const std::string &GetName() const = 0;

    virtual bool IsFullOpaque(Direction direction) const noexcept = 0;

    virtual void AddBlockModel(
        agz::misc::span<std::unique_ptr<ChunkModelBuilder>> modelBuilders,
        const Vec3i &blockPosition, const BlockDescription *neighboringBlocks[6]) const;
};

class BlockDescriptionManager : public Base::Singleton<BlockDescriptionManager>
{
    std::vector<std::shared_ptr<BlockDescription>> blockDescriptions_;
    std::vector<BlockDescription*> rawBlockDescriptions_;

public:

    BlockID RegisterBlockDescription(std::shared_ptr<BlockDescription> desc)
    {
        assert(blockDescriptions_.size() < (std::numeric_limits<BlockID>::max)());
        BlockID id = BlockID(blockDescriptions_.size());
        desc->SetBlockID(id);
        rawBlockDescriptions_.push_back(desc.get());
        blockDescriptions_.push_back(std::move(desc));
        return id;
    }

    const BlockDescription *GetBlockDescription(BlockID id) const noexcept
    {
        assert(id < BlockID(rawBlockDescriptions_.size()));
        return rawBlockDescriptions_[id];
    }

    void Clear()
    {
        blockDescriptions_.clear();
        rawBlockDescriptions_.clear();
    }
};

VRPG_WORLD_END
