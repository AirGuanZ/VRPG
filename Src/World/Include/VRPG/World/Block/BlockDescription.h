#pragma once

#include <cassert>
#include <memory>
#include <vector>

#include <VRPG/Base/Singleton.h>
#include <VRPG/World/Block/BlockBrightness.h>
#include <VRPG/World/Block/BlockOrientation.h>
#include <VRPG/World/Utility/RayBoxIntersect.h>

VRPG_WORLD_BEGIN

class PartialSectionModelBuilder;
class PartialSectionModelBuilderSet;

/**
 * @brief 用于表示单个Block的类型
 */
using BlockID = uint16_t;

/**
 * @brief 方块的某个面具有怎样的可见性判定性质
 *
 * 实体-实体：不可见-不可见
 * 实体-半透明：可见-不可见
 * 实体-镂空：可见-不可见
 * 实体-非box：可见-可见
 * 半透明-半透明：同ID则均不可见，否则均可见
 * 半透明-镂空：可见-可见
 * 半透明-非box：可见-可见
 * 镂空-镂空：根据方向选择其中一个可见，另一个不可见
 * 镂空-非box：可见-可见
 * 非box-非box：可见-可见
 */
enum class FaceVisibilityProperty
{
    Solid       = 0, // 实体，如石头，泥土
    Transparent = 1, // 半透明，如液体底面
    Hollow      = 2, // 镂空，如树叶
    Nonbox      = 3  // 非box类，如围栏
};

/**
 * @brief 方块某个面的可见性
 */
enum class FaceVisibility
{
    Yes,  // 该面可见
    No,   // 该面不可见
    Pos,  // 该面法线为+x/+y/+z时可见，否则不可见
    Diff  // 该面和相邻面ID不同时可见，否则不可见
};

/**
 * @brief 测试一个面是否会被与它相邻的面挡住
 */
inline FaceVisibility IsFaceVisible(FaceVisibilityProperty thisFace, FaceVisibilityProperty neighborFace) noexcept
{
    static const FaceVisibility LUT[4][4] = {
        {
            /* solid-solid             */ FaceVisibility::No,
            /* solid-transparent       */ FaceVisibility::Yes,
            /* solid-hollow            */ FaceVisibility::Yes,
            /* solid-nonbox            */ FaceVisibility::Yes
        },
        {
            /* transparent-solid       */ FaceVisibility::No,
            /* transparent-transparent */ FaceVisibility::Diff,
            /* transparent-hollow      */ FaceVisibility::Yes,
            /* transparent-nonbox      */ FaceVisibility::Yes
        },
        {
            /* hollow-solid            */ FaceVisibility::No,
            /* hollow-transparent      */ FaceVisibility::Yes,
            /* hollow-hollow           */ FaceVisibility::Pos,
            /* hollow-nonbox           */ FaceVisibility::Yes
        },
        {
            /* nonbox-solid            */ FaceVisibility::Yes,
            /* nonbox-transparent      */ FaceVisibility::Yes,
            /* nonbox-hollow           */ FaceVisibility::Yes,
            /* nonbox-nonbox           */ FaceVisibility::Yes
        }
    };
    return LUT[int(thisFace)][int(neighborFace)];
}

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

    virtual FaceVisibilityProperty GetFaceVisibilityProperty(Direction direction) const noexcept = 0;

    virtual bool IsVisible() const noexcept = 0;

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
        PartialSectionModelBuilderSet &modelBuilders,
        const Vec3i &blockPosition,
        const BlockDescription *neighborBlocks[3][3][3],
        const BlockBrightness neighborBrightness[3][3][3],
        const BlockOrientation neighborOrientations[3][3][3]) const = 0;

    /**
     * @brief 射线与方块求交测试
     *
     * 输入为参数化线段 o + t * d (t \in [minT, maxT])，射线位于方块的局部坐标系中（即假设方块位于[0, 1]^3）
     */
    virtual bool RayIntersect(const Vec3 &start, const Vec3 &invDir, float minT, float maxT) const noexcept
    {
        return RayIntersectStdBox(start, invDir, minT, maxT);
    }
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

    FaceVisibilityProperty GetFaceVisibilityProperty(Direction direction) const noexcept override
    {
        return FaceVisibilityProperty::Nonbox;
    }

    bool IsVisible() const noexcept override
    {
        return false;
    }

    bool IsFullOpaque() const noexcept override
    {
        return false;
    }

    void AddBlockModel(
        PartialSectionModelBuilderSet&,
        const Vec3i&, const BlockDescription*[3][3][3], const BlockBrightness[3][3][3], const BlockOrientation[3][3][3]) const override
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

    bool RayIntersect(const Vec3 &start, const Vec3 &invDir, float minT, float maxT) const noexcept override
    {
        return false;
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

        if(auto it = name2Desc_.find(desc->GetName()); it != name2Desc_.end())
        {
            if(it->second != desc)
                throw VRPGWorldException("repeated block description name: " + std::string(desc->GetName()));
            return desc->GetBlockID();
        }

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
