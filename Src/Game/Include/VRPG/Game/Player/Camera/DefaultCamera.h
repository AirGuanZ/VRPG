#pragma once

#include <VRPG/Game/Player/Camera/Camera.h>

VRPG_GAME_BEGIN

class DefaultCamera : public Camera
{
    Vec3 position_;

    float verticalAngle_;
    float horizontalAngle_;

    float nearPlane_;
    float farPlane_;
    float FOVYRad_;
    float wOverH_;

    Mat4 viewMatrix_;
    Mat4 projMatrix_;
    Mat4 viewProjectionMatrix_;
    Vec3 direction_;
    FrustumCuller culler_;

    void Update() noexcept;

public:

    DefaultCamera();

    void SetPosition(const Vec3 &position) noexcept;

    void SetDirection(float verticalAngle, float horizontalAngle) noexcept;

    void SetFOVy(float degree) noexcept;

    void SetWOverH(float wOverH) noexcept;

    void SetClipDistance(float nearPlane, float farPlane) noexcept;

    Mat4 GetViewMatrix() const override;

    Mat4 GetProjMatrix() const override;

    Mat4 GetViewProjectionMatrix() const override;

    Vec3 GetPosition() const noexcept override;

    Vec3 GetDirection() const noexcept override;

    float GetFOVy() const noexcept override;

    float GetWOverH() const noexcept override;

    float GetNearDistance() const noexcept override;

    float GetFarDistance() const noexcept override;

    bool IsVisible(const CullingBoundingBox &bbox) const noexcept override;
};

VRPG_GAME_END
