#pragma once

#include <VRPG/Game/Block/BlockDescription.h>

VRPG_GAME_BEGIN

enum class BuiltinBlockType
{
    Stone,
    Soil,
    Lawn,
    Log,

    GlowStone,

    Leaf,
    Grass,

    WhiteGlass,
    RedGlass,

    Water,

    TypeCount
};

constexpr int BUILTIN_BLOCK_TYPE_COUNT = int(BuiltinBlockType::TypeCount);

class BuiltinBlockTypeManager : public Base::Singleton<BuiltinBlockTypeManager>
{
public:

    struct BuiltinBlockInfo
    {
        std::shared_ptr<const BlockDescription> desc;
    };

    void RegisterBuiltinBlockTypes();

    bool IsAvailable() const noexcept
    {
        return info_[0].desc != nullptr;
    }

    void Clear()
    {
        for(auto &info : info_)
        {
            info.desc.reset();
        }
    }

    const BuiltinBlockInfo &GetInfo(BuiltinBlockType type) const noexcept
    {
        assert(IsAvailable());
        assert(int(type) < BUILTIN_BLOCK_TYPE_COUNT);
        return info_[int(type)];
    }

    const BlockDescription *GetDesc(BuiltinBlockType type) const noexcept
    {
        return GetInfo(type).desc.get();
    }

    BlockID GetID(BuiltinBlockType type) const noexcept
    {
        return GetDesc(type)->GetBlockID();
    }

private:

    std::array<BuiltinBlockInfo, BUILTIN_BLOCK_TYPE_COUNT> info_ = {};
};

VRPG_GAME_END
