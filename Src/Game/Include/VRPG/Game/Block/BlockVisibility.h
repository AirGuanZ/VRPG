#pragma once

#include <VRPG/Game/Common.h>

VRPG_GAME_BEGIN

/**
 * @brief 方块的某个面具有怎样的可见性判定性质
 *
 * 实体-实体：    不可见-不可见
 * 实体-半透明：  可见-可见
 * 实体-镂空：    可见-不可见
 * 实体-非box：   可见-可见
 * 半透明-半透明：同ID则均不可见，否则均可见
 * 半透明-镂空：  可见-可见
 * 半透明-非box： 可见-可见
 * 镂空-镂空：    根据方向选择其中一个可见，另一个不可见
 * 镂空-非box：   可见-可见
 * 非box-非box：  可见-可见
 */
enum class FaceVisibilityProperty
{
    Solid       = 0, // 实体，如石头，泥土
    Transparent = 1, // 半透明，如液体底面
    Hollow      = 2, // 镂空，如树叶
    Nonbox      = 3  // 非box类，如栅栏
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
inline FaceVisibility TestFaceVisibility(FaceVisibilityProperty thisFace, FaceVisibilityProperty neighborFace) noexcept
{
    static const FaceVisibility LUT[4][4] = {
        {
            /* solid-solid             */ FaceVisibility::No,
            /* solid-transparent       */ FaceVisibility::Yes,
            /* solid-hollow            */ FaceVisibility::Yes,
            /* solid-nonbox            */ FaceVisibility::Yes
        },
        {
            /* transparent-solid       */ FaceVisibility::Yes,
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

VRPG_GAME_END
