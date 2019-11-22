#pragma once

#include <cassert>
#include <memory>
#include <vector>

#include <VRPG/Game/World/Block/BlockCollision.h>
#include <VRPG/Game/World/Block/BlockInstance.h>
#include <VRPG/Game/World/Block/BlockVisibility.h>
#include <VRPG/Game/World/Block/LiquidDescription.h>

VRPG_GAME_BEGIN

class ModelBuilderSet;

/**
 * @brief 表示具有相同类型的block的共有属性
 * 
 *  对每个block id值，都有唯一的block description与之对应
 */
class BlockDescription
{
    friend class BlockDescManager;

    BlockID blockID_ = 0;

    void SetBlockID(BlockID id) noexcept { blockID_ = id; }

public:

    virtual ~BlockDescription() = default;

    /**
     * @brief 取得block id，此id通常由block description manager分配
     */
    BlockID GetBlockID() const noexcept { return blockID_; }

    /**
     * @brief 取得block name
     */
    virtual const char *GetName() const = 0;

    /**
     * @brief 取得该方块指定面的可见性类型
     */
    virtual FaceVisibilityType GetFaceVisibility(Direction direction) const noexcept = 0;

    /**
     * @brief 是否是一个可以被液体替代的方块
     *
     * 如空气、水等
     */
    virtual bool IsReplacableByLiquid() const noexcept { return IsVoid() || IsLiquid(); }

    /**
     * @brief 是否是一个不和玩家视线发生碰撞的方块
     */
    virtual bool IsReplacableByCrosshairRay() const noexcept { return IsVoid() || IsLiquid(); }

    /**
     * @brief 该方块是否是一个可见的方块
     */
    virtual bool IsVisible() const noexcept { return true; }

    /**
     * @brief 该方块是否是一个完全遮光的box
     */
    virtual bool IsFullOpaque() const noexcept = 0;

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

    /**
     * @brief 向一个PartialSectionModelBuilderSet追加渲染数据
     *
     * neighboringBlocks[x][y][z]
     * x, y, z in { 0, 1, 2 }
     * 其中[i][j][k]是与被生成的方块相对位置为[i-1][j-1][k-1]的方块
     */
    virtual void AddBlockModel(
        ModelBuilderSet &modelBuilders, const Vec3i &blockPosition, const BlockNeighborhood blocks) const = 0;

    /**
     * @brief 是否需携带额外数据
     */
    virtual bool HasExtraData() const  noexcept;

    /**
     * @brief 创建新额外数据
     */
    virtual BlockExtraData CreateExtraData() const;

    /**
     * @brief 取得碰撞体属性
     * 
     * 默认无碰撞体积
     */
    virtual const BlockCollision *GetCollision() const noexcept;

    /**
     * @brief 取得液体属性
     */
    virtual const LiquidDescription *GetLiquid() const noexcept;

    /**
     * @brief 是否是液体
     */
    bool IsLiquid() const noexcept { return GetLiquid()->isLiquid; }

    /**
     * @brief 是否是void
     */
    bool IsVoid() const noexcept { return blockID_ == BLOCK_ID_VOID; }
};

class BlockDescManager : public Base::Singleton<BlockDescManager>
{
    std::vector<std::shared_ptr<BlockDescription>> blockDescriptions_;
    std::map<std::string, std::shared_ptr<BlockDescription>, std::less<>> name2Desc_;
    std::vector<BlockDescription*> rawBlockDescriptions_;

public:

    BlockDescManager();

    BlockID RegisterBlockDescription(std::shared_ptr<BlockDescription> desc);

    void Clear();

    BlockID GetBlockDescriptionCount() const noexcept
    {
        return BlockID(blockDescriptions_.size());
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
};

VRPG_GAME_END
