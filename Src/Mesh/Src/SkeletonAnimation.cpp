#include <limits>

#include <VRPG/Mesh/SkeletonAnimation.h>
#include <VRPG/Mesh/Utility.h>

VRPG_MESH_BEGIN

BoneAnimation::BoneAnimation(std::vector<BoneTransformKeyFrame> keyframes)
    : keyframes_(std::move(keyframes))
{

}

void BoneAnimation::ComputeTransformMatrix(float t, Mat4 &output) const noexcept
{
    if(keyframes_.empty())
    {
        output = Mat4::identity();
        return;
    }

    t = agz::math::clamp(t, GetStartTime(), GetEndTime());

    // 单独处理t == GetEndTime()的情况

    if(t >= GetEndTime())
    {
        auto &keyframe = keyframes_.back();

        output = Trans4::scale          (keyframe.scale)
               * Trans4::from_quaternion(keyframe.rotate)
               * Trans4::translate      (keyframe.translate);
        return;
    }

    // 找到i使得：keyframe_[i].timePoint <= t < keyframe_[i+1].timePoint
    // IMPROVE: binary search
    
    size_t i = 0;
    for(;;)
    {
        assert(i + 1 < keyframes_.size());
        if(keyframes_[i].timePoint <= t && t < keyframes_[i + 1].timePoint)
        {
            break;
        }
        ++i;
    }

    // 将插值后的变换输出为矩阵

    auto &a = keyframes_[i];
    auto &b = keyframes_[i + 1];
    float interpFactor = (t - a.timePoint) / (b.timePoint - a.timePoint);

    Vec3       interpScale  = lerp (a.scale,     b.scale,     interpFactor);
    Vec3       interpTrans  = lerp (a.translate, b.translate, interpFactor);
    Quaternion interpRotate = slerp(a.rotate,    b.rotate,    interpFactor);

    output = Trans4::scale(interpScale)
           * Trans4::from_quaternion(interpRotate)
           * Trans4::translate(interpTrans);
}

float BoneAnimation::GetStartTime() const noexcept
{
    return keyframes_.empty() ? 0 : keyframes_.front().timePoint;
}

float BoneAnimation::GetEndTime() const noexcept
{
    return keyframes_.empty() ? 0 : keyframes_.back().timePoint;
}

void BoneAnimation::Write(std::ostream &out) const
{
    uint32_t keyframeCount = static_cast<uint32_t>(keyframes_.size());
    WriteOStream(out, keyframeCount);
    WriteOStream(out, keyframes_.data(), sizeof(keyframes_[0]) * keyframes_.size());
}

void BoneAnimation::Read(std::istream &in)
{
    keyframes_.clear();

    uint32_t keyframeCount;
    ReadIStream(in, keyframeCount);

    keyframes_.resize(keyframeCount);
    ReadIStream(in, keyframes_.data(), sizeof(keyframes_[0]) * keyframeCount);
}

SkeletonAnimation::SkeletonAnimation(std::vector<BoneAnimation> boneAnimations)
    : boneAnimations_(std::move(boneAnimations))
{
    if(boneAnimations_.empty())
    {
        startTime_ = endTime_ = 0;
        return;
    }

    startTime_ = boneAnimations_[0].GetStartTime();
    endTime_   = boneAnimations_[0].GetEndTime();
    for(size_t i = 1; i < boneAnimations_.size(); ++i)
    {
        startTime_ = (std::min)(startTime_, boneAnimations_[i].GetStartTime());
        endTime_   = (std::max)(endTime_,   boneAnimations_[i].GetEndTime());
    }
}

void SkeletonAnimation::ComputeTransformMatrix(const StaticSkeleton &staticSkeleton, float t, Mat4 *output) const
{
    assert(staticSkeleton.GetBoneCount());
    assert(staticSkeleton.GetBoneCount() == int(boneAnimations_.size()));

    for(int i = 0; i < staticSkeleton.GetBoneCount(); ++i)
    {
        auto &bone = staticSkeleton.GetBone(i);

        Mat4 aniTrans;
        boneAnimations_[i].ComputeTransformMatrix(t, aniTrans);

        if(bone.parent < 0)
        {
            output[i] = aniTrans;
        }
        else
        {
            assert(bone.parent < i);
            output[i] = aniTrans * output[bone.parent];
        }
    }
}

float SkeletonAnimation::GetStartTime() const noexcept
{
    return startTime_;
}

float SkeletonAnimation::GetEndTime() const noexcept
{
    return endTime_;
}

const BoneAnimation &SkeletonAnimation::GetBoneAnimation(int boneIndex) const noexcept
{
    assert(0 <= boneIndex && boneIndex < int(boneAnimations_.size()));
    return boneAnimations_[boneIndex];
}

void SkeletonAnimation::Write(std::ostream &out) const
{
    uint32_t animationCount = static_cast<uint32_t>(boneAnimations_.size());
    WriteOStream(out, animationCount);
    for(auto &ani : boneAnimations_)
    {
        ani.Write(out);
    }

    WriteOStream(out, startTime_);
    WriteOStream(out, endTime_);
}

void SkeletonAnimation::Read(std::istream &in)
{
    boneAnimations_.clear();

    uint32_t animationCount;
    ReadIStream(in, animationCount);

    boneAnimations_.resize(animationCount);
    for(auto &ani : boneAnimations_)
    {
        ani.Read(in);
    }

    ReadIStream(in, startTime_);
    ReadIStream(in, endTime_);
}

VRPG_MESH_END
