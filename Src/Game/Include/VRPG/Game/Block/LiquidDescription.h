#pragma once

#include <VRPG/Game/Block/BlockInstance.h>

VRPG_GAME_BEGIN

/**
 * @brief 液体级别
 *
 * 液体方块分为1至N+1级，存储在方块实例的附加信息中
 *
 * N+1级表明这是一个液体源，1至N表明这是一个从液体源衍生出来的流动方块
 */
using LiquidLevel = BlockExtraData::uint_t;

/**
 * @brief 从某液体方块的附加信息中取出其液体级别
 *
 * 若输入的附加信息不是来自于一个液体方块，将会造成未定义行为
 */
inline LiquidLevel ExtraDataToLiquidLevel(const BlockExtraData &data) noexcept
{
    return data.get_uint_unchecked();
}

/**
 * @brief 将liquid level值填入一个block extra data中
 */
inline BlockExtraData MakeLiquidExtraData(LiquidLevel level) noexcept
{
    return BlockExtraData(level);
}

/**
 * @brief 液体属性
 */
class LiquidDescription
{
public:

    virtual ~LiquidDescription() = default;

    // 是否是液体，下面的其他成员有意义当且仅当此值为true
    bool isLiquid = false;

    // 液体源头对应的level值，这决定了液体能扩散多远
    LiquidLevel sourceLevel = 7;

    // 液体扩散间隔，决定了液体的扩散速度，是其粘稠度的表征
    StdClock::duration spreadDelay = std::chrono::duration_cast<StdClock::duration>(std::chrono::milliseconds(500));

    // 将液体级别映射为顶点高度，注意不应对液体源调用此方法
    // assert(1 <= level && level <= sourceLevel - 1)
    virtual float LevelToVertexHeight(LiquidLevel level) const noexcept { return std::pow(level / (sourceLevel - 1.0f), 1.5f); }

    // 其上方有不同类型方块的液体源的顶面会比1低一些，此成员函数返回其高度
    virtual float TopSourceHeight() const noexcept { return 0.875f; }

    /**
     * 不同类型的液体之间会发生反应生成新的方块，
     * 此时由id更大的那种液体的desc决定反应的结果
     *
     * @param anotherDesc 与此液体反应的另一液体的类型
     * @param isThisSource 此液体是否为液体源
     * @param isThatSource 彼液体是否为液体源
     *
     * 参数组合表示的含义：
     * - isThisSource  && isThatSource : 一种液体源被放置到另一种液体源时发生
     * - isThisSource  && !isThatSource: 彼液体流动到此液体的液体源
     * - !isThisSource && isThatSource : 此液体流动到彼液体的液体源
     * - !isThisSource && !isThatSource: 两种液体在流动中相遇
     */
    virtual NewBlockInstance ReactWith(const BlockDescription *anotherDesc, bool isThisSource, bool isThatSource) const
    {
        return NewBlockInstance{ BLOCK_ID_DEFAULT, BlockExtraData(), BlockOrientation() };
    }

    /**
     * @brief 判断某个方块实例是否是该液体类型的液体源
     *
     * 要求输入的附加信息必须来自一个此液体方块的实例，否则将造成未定义行为
     */
    bool IsSource(const BlockExtraData &extraData) const noexcept
    {
        return ExtraDataToLiquidLevel(extraData) == sourceLevel;
    }
};

VRPG_GAME_END
