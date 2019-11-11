#pragma once

#include <VRPG/Game/Block/BlockBrightness.h>
#include <VRPG/Game/Block/BlockExtraData.h>
#include <VRPG/Game/Block/BlockOrientation.h>

VRPG_GAME_BEGIN

class BlockDescription;

/**
 * @brief 用于表示单个Block的类型
 */
using BlockID = uint16_t;

/**
 * @brief 空方块拥有固定的ID值
 */
constexpr BlockID BLOCK_ID_VOID = 0;

/**
 * @brief 白方块拥有固定的ID值
 */
constexpr BlockID BLOCK_ID_DEFAULT = 1;

/**
 * @brief 描述单个方块实例所需的全部信息
 */
struct BlockInstance
{
    const BlockDescription *desc = nullptr;
    const BlockExtraData   *extraData = nullptr;
    BlockBrightness         brightness;
    BlockOrientation        orientation;
};

/**
 * @brief 描述一个新生成的方块实例所需的全部信息
 */
struct NewBlockInstance
{
    BlockID id;                   // 方块类型
    BlockExtraData extraData;     // extraData，仅在desc->HasExtraData()为true时有意义
    BlockOrientation orientation; // 方块朝向
};

/**
 * @brief 某位置的方块及其周围的邻居方块
 */
using BlockNeighborhood = BlockInstance[3][3][3];

VRPG_GAME_END
