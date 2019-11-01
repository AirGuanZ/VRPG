#pragma once

#include <VRPG/World/Block/BlockDescription.h>

VRPG_WORLD_BEGIN

enum class BuiltinBlockType
{
    Default,
    Stone,
    Soil,
    Lawn,
    GlowStone,

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

    const BuiltinBlockInfo &GetDesc(BuiltinBlockType type) const noexcept
    {
        assert(IsAvailable());
        assert(int(type) < BUILTIN_BLOCK_TYPE_COUNT);
        return info_[int(type)];
    }

private:

    std::array<BuiltinBlockInfo, BUILTIN_BLOCK_TYPE_COUNT> info_ = {};
};

VRPG_WORLD_END
