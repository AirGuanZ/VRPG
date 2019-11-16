#include <VRPG/Game/Camera/DefaultCamera.h>

VRPG_GAME_BEGIN

namespace
{
    bool IsNegativeForAllVertices(const Vec4 &f, const CullingBoundingBox &bbox) noexcept
    {
        return dot(f, { bbox.low.x,  bbox.low.y,  bbox.low.z,  1 }) < -0.01f &&
               dot(f, { bbox.low.x,  bbox.low.y,  bbox.high.z, 1 }) < -0.01f &&
               dot(f, { bbox.low.x,  bbox.high.y, bbox.low.z,  1 }) < -0.01f &&
               dot(f, { bbox.low.x,  bbox.high.y, bbox.high.z, 1 }) < -0.01f &&
               dot(f, { bbox.high.x, bbox.low.y,  bbox.low.z,  1 }) < -0.01f &&
               dot(f, { bbox.high.x, bbox.low.y,  bbox.high.z, 1 }) < -0.01f &&
               dot(f, { bbox.high.x, bbox.high.y, bbox.low.z,  1 }) < -0.01f &&
               dot(f, { bbox.high.x, bbox.high.y, bbox.high.z, 1 }) < -0.01f;
    }
}

void DefaultCamera::Update() noexcept
{
    direction_ = {
        std::cos(verticalAngle_) * std::cos(horizontalAngle_),
        std::sin(verticalAngle_),
        std::cos(verticalAngle_) * std::sin(horizontalAngle_)
    };
    viewMatrix_ = Trans4::look_at(position_, position_ + direction_, Vec3(0, 1, 0));
    projMatrix_ = Trans4::perspective(FOVYRad_, wOverH_, nearPlane_, farPlane_);
    viewProjectionMatrix_ = viewMatrix_ * projMatrix_;

    cullingF_[0] = viewProjectionMatrix_.get_col(3) - viewProjectionMatrix_.get_col(0);
    cullingF_[1] = viewProjectionMatrix_.get_col(3) + viewProjectionMatrix_.get_col(0);
    cullingF_[2] = viewProjectionMatrix_.get_col(3) - viewProjectionMatrix_.get_col(1);
    cullingF_[3] = viewProjectionMatrix_.get_col(3) + viewProjectionMatrix_.get_col(1);
    cullingF_[4] = viewProjectionMatrix_.get_col(3);
}

DefaultCamera::DefaultCamera()
    : cursorHistoryX_(4), cursorHistoryY_(4)
{
    verticalAngle_ = 0;
    horizontalAngle_ = 0;

    moveSpeed_ = 0;
    viewSpeed_ = 0;

    nearPlane_ = 0.01f;
    farPlane_ = 100.0f;
    FOVYRad_ = agz::math::deg2rad(60.0f);
    wOverH_ = 1;

    Update();
}

void DefaultCamera::SetPosition(const Vec3 &position) noexcept
{
    position_ = position;
    Update();
}

void DefaultCamera::SetDirection(float verticalAngle, float horizontalAngle) noexcept
{
    verticalAngle_ = verticalAngle;
    horizontalAngle_ = horizontalAngle;
    Update();
}

void DefaultCamera::SetFOVy(float degree) noexcept
{
    FOVYRad_ = agz::math::deg2rad(degree);
    Update();
}

void DefaultCamera::SetWOverH(float wOverH) noexcept
{
    wOverH_ = wOverH;
    Update();
}

void DefaultCamera::SetClipDistance(float nearPlane, float farPlane) noexcept
{
    nearPlane_ = nearPlane;
    farPlane_ = farPlane;
    Update();
}

void DefaultCamera::SetMoveSpeed(float unitPerSecond) noexcept
{
    moveSpeed_ = unitPerSecond;
}

void DefaultCamera::SetViewSpeed(float radPerPixel) noexcept
{
    viewSpeed_ = radPerPixel;
}

Vec3 DefaultCamera::GetPosition() const noexcept
{
    return position_;
}

Vec3 DefaultCamera::GetDirection() const noexcept
{
    return direction_;
}

float DefaultCamera::GetFOVy() const noexcept
{
    return FOVYRad_;
}

float DefaultCamera::GetWOverH() const noexcept
{
    return wOverH_;
}

float DefaultCamera::GetNearDistance() const noexcept
{
    return nearPlane_;
}

float DefaultCamera::GetFarDistance() const noexcept
{
    return farPlane_;
}

void DefaultCamera::Update(const Input &input, float deltaT) noexcept
{
    // 视角转动

    cursorHistoryX_.Update(input.relativeCursorX);
    cursorHistoryY_.Update(input.relativeCursorY);

    verticalAngle_ -= viewSpeed_ * cursorHistoryY_.MeanValue();
    horizontalAngle_ -= viewSpeed_ * cursorHistoryX_.MeanValue();

    constexpr float VERTICAL_ANGLE_LIMIT = 0.5f * agz::math::PI_f - 0.01f;
    verticalAngle_ = agz::math::clamp(verticalAngle_, -VERTICAL_ANGLE_LIMIT, VERTICAL_ANGLE_LIMIT);

    // 位置移动

    int rightMovement = 0, frontMovement = 0, upMovement = 0;
    if(input.front) ++frontMovement;
    if(input.back)  --frontMovement;
    if(input.right) ++rightMovement;
    if(input.left)  --rightMovement;
    if(input.up)    ++upMovement;
    if(input.down)  --upMovement;

    if(rightMovement || frontMovement || upMovement)
    {
        Vec3 frontDirection(std::cos(horizontalAngle_), 0, std::sin(horizontalAngle_));
        Vec3 rightDirection(frontDirection.z, 0, -frontDirection.x);
        Vec3 direction =
            float(frontMovement) * frontDirection
          + float(rightMovement) * rightDirection
          + float(upMovement) * Vec3(0, 1, 0);
        position_ += moveSpeed_ * direction.normalize() * deltaT / 1000.0f;
    }

    Update();
    Update();
}

Base::Mat4 DefaultCamera::GetViewMatrix() const
{
    return viewMatrix_;
}

Base::Mat4 DefaultCamera::GetProjMatrix() const
{
    return projMatrix_;
}

Mat4 DefaultCamera::GetViewProjectionMatrix() const
{
    return viewProjectionMatrix_;
}

bool DefaultCamera::IsVisible(const CullingBoundingBox &bbox) const noexcept
{
    return !IsNegativeForAllVertices(cullingF_[0], bbox) &&
           !IsNegativeForAllVertices(cullingF_[1], bbox) &&
           !IsNegativeForAllVertices(cullingF_[2], bbox) &&
           !IsNegativeForAllVertices(cullingF_[3], bbox) &&
           !IsNegativeForAllVertices(cullingF_[4], bbox);
}

VRPG_GAME_END
