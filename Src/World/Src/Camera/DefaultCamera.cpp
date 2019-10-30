#include <VRPG/World/Camera/DefaultCamera.h>

VRPG_WORLD_BEGIN

void DefaultCamera::UpdateViewProjectionMatrix() const noexcept
{
    Vec3 direction(
        std::cos(verticalAngle_) * std::cos(horizontalAngle_),
        std::sin(verticalAngle_),
        std::cos(verticalAngle_) * std::sin(horizontalAngle_));
    Mat4 viewMatrix = Trans4::look_at(position_, position_ + direction, Vec3(0, 1, 0));
    Mat4 projMatrix = Trans4::perspective(FOVYRad_, wOverH_, nearPlane_, farPlane_);
    viewProjectionMatrix_ = viewMatrix * projMatrix;
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

    UpdateViewProjectionMatrix();
    isMatrixDirty_ = false;
}

void DefaultCamera::SetPosition(const Vec3 &position) noexcept
{
    position_ = position;
    isMatrixDirty_ = true;
}

void DefaultCamera::SetDirection(float verticalAngle, float horizontalAngle) noexcept
{
    verticalAngle_ = verticalAngle;
    horizontalAngle_ = horizontalAngle;
    isMatrixDirty_ = true;
}

void DefaultCamera::SetFOVy(float degree) noexcept
{
    FOVYRad_ = agz::math::deg2rad(degree);
    isMatrixDirty_ = true;
}

void DefaultCamera::SetWOverH(float wOverH) noexcept
{
    wOverH_ = wOverH;
    isMatrixDirty_ = true;
}

void DefaultCamera::SetClipDistance(float nearPlane, float farPlane) noexcept
{
    nearPlane_ = nearPlane;
    farPlane_ = farPlane;
    isMatrixDirty_ = true;
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
    return Vec3(
            std::cos(verticalAngle_) * std::cos(horizontalAngle_),
            std::sin(verticalAngle_),
            std::cos(verticalAngle_) * std::sin(horizontalAngle_));
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

    UpdateViewProjectionMatrix();
    isMatrixDirty_ = false;
}

Mat4 DefaultCamera::GetViewProjectionMatrix() const
{
    if(isMatrixDirty_)
    {
        UpdateViewProjectionMatrix();
        isMatrixDirty_ = false;
    }
    return viewProjectionMatrix_;
}

VRPG_WORLD_END
