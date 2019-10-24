#pragma once

#include <cassert>
#include <memory>
#include <vector>

#include <VRPG/Base/Singleton.h>
#include <VRPG/World/Common.h>

VRPG_WORLD_BEGIN

class PartialSectionModelBuilder;

enum Direction
{
    PositiveX = 0,
    NegativeX = 1,
    PositiveZ = 2,
    NegativeZ = 3,
    PositiveY = 4,
    NegativeY = 5
};

using BlockID = uint16_t;

class BlockDescription
{
    BlockID blockID_ = 0;

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

    virtual bool IsVisible() const noexcept = 0;

    virtual void AddBlockModel(
        agz::misc::span<std::unique_ptr<PartialSectionModelBuilder>> modelBuilders,
        const Vec3i &blockPosition, const BlockDescription *neighboringBlocks[6]) const = 0;

    /**
     * @brief 本方块是否是一个光源
     */
    virtual bool IsLightSource() const noexcept = 0;

    /**
     * @brief 光照传播到该block时的衰减量
     */
    virtual BlockBrightness LightAttenuation() const noexcept = 0;

    /**
     * @brief 本方块的自发光
     */
    virtual BlockBrightness InitialBrightness() const noexcept = 0;
};

class VoidBlockDescription : public BlockDescription
{
public:

    const std::string &GetName() const override
    {
        static const std::string ret = "void";
        return ret;
    }

    bool IsFullOpaque(Direction) const noexcept override
    {
        return false;
    }

    bool IsVisible() const noexcept override
    {
        return false;
    }

    void AddBlockModel(
        agz::misc::span<std::unique_ptr<PartialSectionModelBuilder>>,
        const Vec3i&, const BlockDescription**) const override
    {
        // do nothing
    }

    bool IsLightSource() const noexcept override
    {
        return false;
    }

    BlockBrightness LightAttenuation() const noexcept override
    {
        return { 1, 1, 1, 1 };
    }

    BlockBrightness InitialBrightness() const noexcept override
    {
        return { 0, 0, 0, 0 };
    }
};

constexpr BlockID BLOCK_ID_VOID = 0;

class BlockDescriptionManager : public Base::Singleton<BlockDescriptionManager>
{
    std::vector<std::shared_ptr<BlockDescription>> blockDescriptions_;
    std::vector<BlockDescription*> rawBlockDescriptions_;

public:

    BlockDescriptionManager()
    {
        auto voidDesc = std::make_shared<VoidBlockDescription>();
        RegisterBlockDescription(std::move(voidDesc));
    }

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
        blockDescriptions_.resize(1);
        rawBlockDescriptions_.resize(1);
    }
};

VRPG_WORLD_END
