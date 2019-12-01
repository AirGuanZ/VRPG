#include <VRPG/Game/Player/Camera/DefaultCamera.h>

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

    culler_ = FrustumCuller(viewProjectionMatrix_);
}

DefaultCamera::DefaultCamera()
{
    verticalAngle_   = 0;
    horizontalAngle_ = 0;

    nearPlane_ = 0.01f;
    farPlane_  = 100.0f;
    FOVYRad_   = agz::math::deg2rad(60.0f);
    wOverH_    = 1;

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
    return culler_.IsVisible(bbox);
}

VRPG_GAME_END
