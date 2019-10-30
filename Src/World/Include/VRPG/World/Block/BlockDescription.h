#pragma once

#include <cassert>
#include <memory>
#include <vector>

#include <VRPG/Base/Singleton.h>
#include <VRPG/World/Block/BlockBrightness.h>

VRPG_WORLD_BEGIN

class PartialSectionModelBuilder;
class PartialSectionModelBuilderSet;

/**
 * @brief 用于表示单个Block的类型
 */
using BlockID = uint16_t;

/**
 * @brief 表示具有相同类型的block的共有属性
 * 
 *  对每个block id值，都有唯一的block description与之对应
 */
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

    virtual const char *GetName() const = 0;

    virtual bool IsFullOpaque(Direction direction) const noexcept = 0;

    virtual bool IsVisible() const noexcept = 0;

    /**
     * @brief 向一个PartialSectionModelBuilderSet追加渲染数据
     *
     * neighboringBlocks[x][y][z]
     * x, y, z in { 0, 1, 2 }
     * 其中[i][j][k]是与被生成的方块相对位置为[i-1][j-1][k-1]的方块
     */
    virtual void AddBlockModel(
        PartialSectionModelBuilderSet &modelBuilders,
        const Vec3i &blockPosition,
        const BlockDescription *neighboringBlocks[3][3][3],
        BlockBrightness neighboringBrightness[3][3][3]) const = 0;

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

/**
 * @brief 空方块
 */
class VoidBlockDescription : public BlockDescription
{
public:

    const char *GetName() const override
    {
        return "void";
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
        PartialSectionModelBuilderSet&,
        const Vec3i&, const BlockDescription*[3][3][3], BlockBrightness[3][3][3]) const override
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

/**
 * @brief 空方块拥有固定的ID值
 */
constexpr BlockID BLOCK_ID_VOID = 0;

class BlockDescriptionManager : public Base::Singleton<BlockDescriptionManager>
{
    std::vector<std::shared_ptr<BlockDescription>> blockDescriptions_;
    std::map<std::string, std::shared_ptr<BlockDescription>, std::less<>> name2Desc_;
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
        spdlog::info("register block description (name = {}, id = {})", desc->GetName(), id);

        rawBlockDescriptions_.push_back(desc.get());
        name2Desc_[std::string(desc->GetName())] = desc;
        blockDescriptions_.push_back(std::move(desc));
        return id;
    }

    const BlockDescription *GetBlockDescription(BlockID id) const noexcept
    {
        assert(id < BlockID(rawBlockDescriptions_.size()));
        return rawBlockDescriptions_[id];
    }

    const BlockDescription *GetBlockDescriptionByName(std::string_view name) const
    {
        auto it = name2Desc_.find(name);
        return it != name2Desc_.end() ? it->second.get() : nullptr;
    }

    void Clear()
    {
        blockDescriptions_.clear();
        name2Desc_.clear();
        rawBlockDescriptions_.clear();

        auto voidDesc = std::make_shared<VoidBlockDescription>();
        voidDesc->SetBlockID(BLOCK_ID_VOID);

        rawBlockDescriptions_.push_back(voidDesc.get());
        name2Desc_[std::string(voidDesc->GetName())] = voidDesc;
        blockDescriptions_.push_back(std::move(voidDesc));
    }
};

VRPG_WORLD_END
