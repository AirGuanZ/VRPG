#pragma once

#include <VRPG/Game/Common.h>

#include <VRPG/Mesh/Mesh.h>

VRPG_GAME_BEGIN

class AnimationModel
{
public:

    using Skeleton     = Mesh::StaticSkeleton;
    using Animation    = Mesh::SkeletonAnimation;
    using AnimationMap = std::map<std::string, Animation, std::less<>>;

    AnimationModel() = default;

    AnimationModel(Skeleton staticSkeleton, AnimationMap animationMap);

    void Initialize(Skeleton staticSkeleton, AnimationMap animationMap);

    int GetBoneCount() const noexcept;

    void ComputeStaticTransform(Mat4 *output) const;

    const Skeleton &GetSkeleton() const noexcept;

    const Animation *GetAnimation(std::string_view name) const;

private:

    Mesh::StaticSkeleton staticSkeleton_;
    AnimationMap         animationMap_;
};

inline AnimationModel::AnimationModel(Mesh::StaticSkeleton staticSkeleton, AnimationMap animationMap)
{
    Initialize(std::move(staticSkeleton), std::move(animationMap));
}

inline void AnimationModel::Initialize(Skeleton staticSkeleton, AnimationMap animationMap)
{
    staticSkeleton_ = std::move(staticSkeleton);
    animationMap_   = std::move(animationMap);
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

VRPG_GAME_END
