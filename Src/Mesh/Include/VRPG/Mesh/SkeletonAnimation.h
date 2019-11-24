#pragma once

#include <vector>

#include <VRPG/Mesh/StaticSkeleton.h>

VRPG_MESH_BEGIN

/**
 * @brief 单个骨骼的单帧变换
 */
struct BoneTransformKeyFrame
{
    float timePoint = 0;

    Vec3       translate;
    Vec3       scale;
    Quaternion rotate;
};

/**
 * @brief 单个骨骼的单个动画
 */
class BoneAnimation
{
public:

    /**
     * 传入空关键帧序列表示没有动画，此时transform matrix始终为identity
     */
    explicit BoneAnimation(std::vector<BoneTransformKeyFrame> keyframes = {});

    void ComputeTransformMatrix(float t, Mat4 &output) const noexcept;

    float GetStartTime() const noexcept;

    float GetEndTime() const noexcept;

    void Write(std::ostream &out) const;

    void Read(std::istream &in);

private:

    std::vector<BoneTransformKeyFrame> keyframes_;
};

/**
 * @brief 骨架的单个动画
 */
class SkeletonAnimation
{
public:

    explicit SkeletonAnimation(std::vector<BoneAnimation> boneAnimations);

    void ComputeTransformMatrix(const StaticSkeleton &staticSkeleton, float t, Mat4 *output) const;

    float GetStartTime() const noexcept;

    float GetEndTime() const noexcept;

    void Write(std::ostream &out) const;

    void Read(std::istream &in);

private:

    std::vector<BoneAnimation> boneAnimations_;

    float startTime_, endTime_;
};

VRPG_MESH_END
