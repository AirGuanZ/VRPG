#pragma once

#include <VRPG/Mesh/Common.h>

VRPG_MESH_BEGIN

// 骨骼动画

struct BoneTransformKeyFrame
{
    float timePoint = 0;

    Vec3       translate;
    Vec3       scale;
    Quaternion rotate;
};

class BoneAnimationClip
{
public:

    explicit BoneAnimationClip(std::vector<BoneTransformKeyFrame> keyframes);

    void ComputeTransformMatrix(float t, Mat4 &output) const noexcept;

    float GetStartTime() const noexcept;

    float GetEndTime() const noexcept;

private:

    std::vector<BoneTransformKeyFrame> keyframes_;
    float startTime_, endTime_;
};

class SkeletonAnimationClip
{
public:

    explicit SkeletonAnimationClip(std::vector<BoneAnimationClip> boneAnimations);

    void ComputeTransformMatrix(float t, Mat4 *output) const noexcept;

    float GetStartTime() const noexcept;

    float GetEndTime() const noexcept;

private:

    std::vector<BoneAnimationClip> boneAnimations_;
    float startTime_, endTime_;
};

class SkeletonAnimation
{
public:

    using Map = std::map<std::string, SkeletonAnimationClip, std::less<>>;

    void AddClip(std::string name, SkeletonAnimationClip clipData);

    void InitializeTree(std::vector<int> parents);

    bool ComputeTransformMatrix(std::string_view clipName, float t, Mat4 *output) const;

    const SkeletonAnimationClip *GetAnimationClip(std::string_view clipName) const;

    const Map &GetAllAnimationClips() const noexcept;

    const std::vector<int> &GetTree() const noexcept;

    SkeletonAnimation Scale(float timeRatio, float sizeRatio);

private:

    std::vector<int> tree_;
    Map              animationClips_;
};

VRPG_MESH_END
