#pragma once

#include <memory>

#include <agz/utility/misc.h>

#include <VRPG/Game/Common.h>

VRPG_GAME_BEGIN

/**
 * 有的方块的附加信息不能简单地用一个uint表示，
 * 此时使用一个专门的对象，此对象必继承自BlockExtraDataObject
 */
class BlockExtraDataObject
{
public:

    virtual ~BlockExtraDataObject() = default;

    // for agz::misc::uint_ptr_variant
    virtual std::unique_ptr<BlockExtraDataObject> clone() const = 0;
};

/**
 * @brief 方块附加信息
 *
 * 不是所有的方块都有附加信息
 */
using BlockExtraData = agz::misc::uint_ptr_variant_t<uint16_t, BlockExtraDataObject>;

VRPG_GAME_END
