#pragma once

#include <cassert>
#include <memory>
#include <vector>

#include <VRPG/Base/Singleton.h>
#include <VRPG/Game/Chunk/ChunkModel.h>

VRPG_GAME_BEGIN

using BlockEffectID = uint16_t;

/**
 * @brief 通用区块渲染参数
 * 
 * 所有的block effect在渲染时均只接收此参数
 */
struct BlockRenderParams
{
    const Camera *camera;
    const Vec3 skyLight;
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

    virtual const char *GetName() const = 0;

    virtual bool IsTransparent() const noexcept = 0;

    BlockEffectID GetBlockEffectID() const noexcept { return blockEffectID_; }

    virtual void Bind() const = 0;

    virtual void Unbind() const = 0;

    virtual std::unique_ptr<PartialSectionModelBuilder> CreateModelBuilder(const Vec3i &globalSectionPosition) const = 0;

    virtual void SetRenderParams(const BlockRenderParams &params) const = 0;
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
