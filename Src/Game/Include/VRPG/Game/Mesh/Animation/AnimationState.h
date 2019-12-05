#pragma once

#include <VRPG/Game/Mesh/Animation/AnimationModel.h>

VRPG_GAME_BEGIN

/**
 * @brief 骨骼动画的播放状态
 */
class AnimationState
{
public:

    AnimationState() = default;

    explicit AnimationState(std::shared_ptr<const AnimationModel> model);

    void Initialize(std::shared_ptr<const AnimationModel> model);

    /**
     * @brief 设置当前播放的动画名
     *
     * 播放时间会自动清零
     *
     * 若不存在具有该名字的动画，会切换到静态骨骼
     */
    void SetCurrentAnimationName(std::string_view animationName);

    /**
     * @brief 取得当前正在播放的动画
     *
     * 无正播放的动画时返回nullptr
     */
    const AnimationModel::Animation *GetCurrentAnimation() const noexcept;

    /**
     * @brief 取得当前播放动画的时间点
     */
    float GetCurrentAnimationTime() const noexcept;

    /**
     * @brief 设置当前播放动画的时间点
     *
     * 若开启了循环播放，超出当前动画结束时间的值会对结束时间取余
     */
    void SetCurrentAnimationTime(float timePoint) noexcept;

    /**
     * @brief 是否开启了循环播放
     */
    bool IsAnimationLoopEnabled() const noexcept;

    /**
     * @brief 开启或关闭循环播放
     */
    void EnableAnimationLoop(bool enabled) noexcept;

    /**
     * @brief 当前动画是否已经播放完毕
     */
    bool IsAnimationEnd() const noexcept;

    /**
     * @brief 取得骨骼数量
     */
    int GetBoneCount() const noexcept;

    /**
     * @brief 计算所有骨骼的world transform
     *
     * output应包含至少GetBoundCount()个Mat4
     */
    void ComputeTransform(Mat4 *output) const;

private:

    std::shared_ptr<const AnimationModel> model_;

    const AnimationModel::Animation *animation_     = nullptr;
    float                            animationTime_ = 0;
    bool                             enableLoop_    = false;
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
