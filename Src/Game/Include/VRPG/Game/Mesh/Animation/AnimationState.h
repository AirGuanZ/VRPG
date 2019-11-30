#pragma once

#include <VRPG/Game/Mesh/Animation/AnimationModel.h>

VRPG_GAME_BEGIN

class AnimationState
{
public:

    AnimationState() = default;

    explicit AnimationState(std::shared_ptr<const AnimationModel> model);

    void Initialize(std::shared_ptr<const AnimationModel> model);

    void SetCurrentAnimationName(std::string_view animationName);

    const AnimationModel::Animation *GetCurrentAnimation() const noexcept;

    float GetCurrentAnimationTime() const noexcept;

    void SetCurrentAnimationTime(float timePoint) noexcept;

    bool IsAnimationLoopEnabled() const noexcept;

    void EnableAnimationLoop(bool enabled) noexcept;

    bool IsAnimationEnd() const noexcept;

    int GetBoneCount() const noexcept;

    void ComputeTransform(Mat4 *output) const;

private:

    std::shared_ptr<const AnimationModel> model_;

    const AnimationModel::Animation *animation_;
    float                            animationTime_;
    bool                             enableLoop_;
};

inline AnimationState::AnimationState(std::shared_ptr<const AnimationModel> model)
    : model_(std::move(model))
{
    animation_     = nullptr;
    animationTime_ = 0;
    enableLoop_    = false;
}

inline void AnimationState::Initialize(std::shared_ptr<const AnimationModel> model)
{
    model_         = std::move(model);
    animation_     = nullptr;
    animationTime_ = 0;
    enableLoop_    = false;
}

inline void AnimationState::SetCurrentAnimationName(std::string_view animationName)
{
    animation_     = model_->GetAnimation(animationName);
    animationTime_ = 0;
}

inline const AnimationModel::Animation *AnimationState::GetCurrentAnimation() const noexcept
{
    return animation_;
}

inline void AnimationState::SetCurrentAnimationTime(float timePoint) noexcept
{
    animationTime_ = timePoint;
    if(!animation_)
    {
        return;
    }

    float endTime = animation_->GetEndTime();
    if(animationTime_ > endTime)
    {
        if(enableLoop_)
        {
            while(animationTime_ > endTime)
            {
                animationTime_ -= endTime;
            }
        }
        else
        {
            animationTime_ = endTime;
        }
    }
}

inline float AnimationState::GetCurrentAnimationTime() const noexcept
{
    return animationTime_;
}

inline bool AnimationState::IsAnimationLoopEnabled() const noexcept
{
    return enableLoop_;
}

inline void AnimationState::EnableAnimationLoop(bool enabled) noexcept
{
    enableLoop_ = enabled;
}

inline bool AnimationState::IsAnimationEnd() const noexcept
{
    if(!animation_)
    {
        return false;
    }
    return animationTime_ > animation_->GetEndTime();
}

inline int AnimationState::GetBoneCount() const noexcept
{
    return model_->GetBoneCount();
}

inline void AnimationState::ComputeTransform(Mat4 *output) const
{
    if(animation_)
    {
        animation_->ComputeTransformMatrix(model_->GetSkeleton(), animationTime_, output);
    }
    else
    {
        model_->ComputeStaticTransform(output);
    }
}

VRPG_GAME_END
