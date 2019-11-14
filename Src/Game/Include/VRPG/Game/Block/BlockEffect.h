#pragma once

#include <cassert>
#include <memory>
#include <vector>

#include <VRPG/Base/Singleton.h>
#include <VRPG/Game/Chunk/ChunkModel.h>

VRPG_GAME_BEGIN

using BlockEffectID = uint16_t;

/**
 * @brief 通用区块前向渲染参数
 * 
 * 所有的block effect在前向渲染时均只接收此参数
 */
struct BlockForwardRenderParams
{
    const Camera *camera = nullptr;
    Vec3 skyLight;
    Vec3 sunlightDirection;

    float shadowScale = 1;
    Mat4 shadowViewProj;
    ComPtr<ID3D11ShaderResourceView> shadowMapSRV;

    float dx = 1.0f / 4096;
};

/**
 * @brief 通用区块shadow map渲染参数
 */
struct BlockShadowRenderParams
{
    Mat4 shadowViewProj;
};

/*
一个BlockEffect代表一种特定类型的方块外观，包括其shader、纹理、input layout等

一个chunk model由多个section model构成，一个section model包含多个partial section model

每个partial section model对应一种block effect
*/
class BlockEffect
{
    friend class BlockEffectManager;

    BlockEffectID blockEffectID_ = 0;

    void SetBlockEffectID(BlockEffectID id) noexcept { blockEffectID_ = id; }

public:

    virtual ~BlockEffect() = default;

    /**
     * @brief 取得该Effect实例的名字
     *
     * 原则上每个BlockEffect实例都有一个独有的名字
     */
    virtual const char *GetName() const = 0;

    /**
     * @brief 是否是透明物体
     *
     * 透明物体将晚于其他物体渲染，且需要排序
     */
    virtual bool IsTransparent() const noexcept = 0;

    /**
     * @brief 取得BlockEffectID
     *
     * 此ID由BlockEffectManager自动分配
     */
    BlockEffectID GetBlockEffectID() const noexcept { return blockEffectID_; }

    /**
     * @brief 开始以此effect进行前向渲染
     */
    virtual void StartForward() const = 0;

    /**
     * @brief 结束以此effect进行前向渲染
     */
    virtual void EndForward() const = 0;

    /**
     * @brief 开始以此effect进行shadow map渲染
     *
     * TODO: 删除默认实现
     */
    virtual void StartShadow() const { }

    /**
     * @brief 结束以此effect进行shadow map渲染
     */
    virtual void EndShadow() const { }

    /**
     * @brief 创建一个空的、用于构建具有此effect的chunk section model的model builder
     */
    virtual std::unique_ptr<PartialSectionModelBuilder> CreateModelBuilder(const Vec3i &globalSectionPosition) const = 0;

    /**
     * @brief 设置前向渲染参数
     */
    virtual void SetForwardRenderParams(const BlockForwardRenderParams &params) const = 0;

    /**
     * @brief 设置shadow map渲染参数
     *
     * TODO: 删除默认实现
     */
    virtual void SetShadowRenderParams(const BlockShadowRenderParams &params) const { }
};

/**
 * @brief default block effect的固定ID
 */
constexpr BlockEffectID BLOCK_EFFECT_ID_DEFAULT = 0;

class BlockEffectManager : public Base::Singleton<BlockEffectManager>
{
    std::vector<std::shared_ptr<const BlockEffect>> blockEffects_;
    std::map<std::string, std::shared_ptr<const BlockEffect>, std::less<>> name2Effect_;
    std::vector<BlockEffect*> rawBlockEffects_;

public:

    BlockEffectManager();

    BlockEffectID RegisterBlockEffect(std::shared_ptr<BlockEffect> effect);

    void Clear();

    size_t GetBlockEffectCount() const noexcept
    {
        return blockEffects_.size();
    }

    const BlockEffect *GetBlockEffect(BlockEffectID id) const noexcept
    {
        assert(id < BlockEffectID(blockEffects_.size()));
        return rawBlockEffects_[id];
    }

    const std::shared_ptr<const BlockEffect> &GetSharedBlockEffect(BlockEffectID id) const noexcept
    {
        assert(id < BlockEffectID(blockEffects_.size()));
        return blockEffects_[id];
    }

    const BlockEffect *GetBlockEffectByName(std::string_view name) const
    {
        auto it = name2Effect_.find(name);
        return it != name2Effect_.end() ? it->second.get() : nullptr;
    }

    const std::shared_ptr<const BlockEffect> &GetSharedBlockEffectByName(std::string_view name) const
    {
        return GetSharedBlockEffect(GetBlockEffectByName(name)->GetBlockEffectID());
    }
};

/**
 * @brief 用于生成partial section model的辅助设施
 * 
 * 包含所有block effect对应的partial section model builder的实例
 */
class PartialSectionModelBuilderSet
{
    std::vector<std::unique_ptr<PartialSectionModelBuilder>> builders_;

public:

    PartialSectionModelBuilderSet(const Vec3i &globalSectionModel)
    {
        auto &effectMgr = BlockEffectManager::GetInstance();
        BlockEffectID blockEffectCount = BlockEffectID(effectMgr.GetBlockEffectCount());
        builders_.reserve(effectMgr.GetBlockEffectCount());
        for(BlockEffectID i = 0; i < blockEffectCount; ++i)
        {
            const BlockEffect *effect = effectMgr.GetSharedBlockEffect(i).get();
            builders_.push_back(effect->CreateModelBuilder(globalSectionModel));
        }
    }

    template<typename Effect>
    auto GetBuilderByEffect(const Effect *effect)
    {
        using Builder = typename Effect::Builder;
        static_assert(std::is_base_of_v<BlockEffect, Effect>);
        static_assert(std::is_base_of_v<PartialSectionModelBuilder, Builder>);

        BlockEffectID id = effect->GetBlockEffectID();
        assert(0 <= id && id < builders_.size());

        PartialSectionModelBuilder *rawBuilder = builders_[id].get();
        Builder *builder = dynamic_cast<Builder*>(rawBuilder);
        assert(builder);
        return builder;
    }

    auto begin() { return builders_.begin(); }
    auto end()   { return builders_.end();   }

    auto begin() const { return builders_.begin(); }
    auto end()   const { return builders_.end();   }
};

VRPG_GAME_END
