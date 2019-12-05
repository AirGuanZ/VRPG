#pragma once

#include <VRPG/Game/Common.h>

#include <VRPG/Mesh/Mesh.h>

VRPG_GAME_BEGIN

/**
 * @brief 骨骼动画数据
 */
class AnimationModel : public agz::misc::uncopyable_t
{
public:

    using Skeleton     = Mesh::StaticSkeleton;
    using Animation    = Mesh::SkeletonAnimation;
    using AnimationMap = std::map<std::string, Animation, std::less<>>;

    static std::shared_ptr<AnimationModel> CreateAnimationModel(Skeleton staticSkeleton, AnimationMap animationMap);

    /**
     * @brief 取得骨骼数量
     */
    int GetBoneCount() const noexcept;

    /**
     * @brief 取得静态骨骼变换
     *
     * output应包含至少GetBoundCount()个Mat4
     */
    void ComputeStaticTransform(Mat4 *output) const;

    /**
     * @brief 取得静态骨骼
     */
    const Skeleton &GetSkeleton() const noexcept;

    /**
     * @brief 取得具有指定名字的动画
     *
     * 不存在拥有该名字的动画时返回nullptr
     */
    const Animation *GetAnimation(std::string_view name) const;

private:

    AnimationModel(Skeleton staticSkeleton, AnimationMap animationMap);

    Mesh::StaticSkeleton staticSkeleton_;
    AnimationMap         animationMap_;
};

inline std::shared_ptr<AnimationModel> AnimationModel::CreateAnimationModel(Skeleton staticSkeleton, AnimationMap animationMap)
{
    auto ptr = new AnimationModel(std::move(staticSkeleton), std::move(animationMap));
    return std::shared_ptr<AnimationModel>(ptr);
}

inline int AnimationModel::GetBoneCount() const noexcept
{
    return staticSkeleton_.GetBoneCount();
}

inline void AnimationModel::ComputeStaticTransform(Mat4 *output) const
{
    assert(output);
    staticSkeleton_.ComputeStaticTransformMatrix(output);
}

inline const AnimationModel::Skeleton &AnimationModel::GetSkeleton() const noexcept
{
    return staticSkeleton_;
}

inline const AnimationModel::Animation *AnimationModel::GetAnimation(std::string_view name) const
{
    auto it = animationMap_.find(name);
    return it != animationMap_.end() ? &it->second : nullptr;
}

inline AnimationModel::AnimationModel(Mesh::StaticSkeleton staticSkeleton, AnimationMap animationMap)
{
    staticSkeleton_ = std::move(staticSkeleton);
    animationMap_   = std::move(animationMap);
}

VRPG_GAME_END
