#pragma once

#include <VRPG/Game/Camera/Camera.h>
#include <VRPG/Game/Utility/ScalarHistory.h>

VRPG_GAME_BEGIN

class DefaultCamera : public Camera
{
    ScalarHistory cursorHistoryX_;
    ScalarHistory cursorHistoryY_;

    Vec3 position_;

    float verticalAngle_;
    float horizontalAngle_;

    float moveSpeed_;
    float viewSpeed_;

    float nearPlane_;
    float farPlane_;
    float FOVYRad_;
    float wOverH_;

    Mat4 viewMatrix_;
    Mat4 projMatrix_;
    Mat4 viewProjectionMatrix_;
    Vec4 cullingF_[5];
    Vec3 direction_;

    void Update() noexcept;

public:

    struct Input
    {
        bool left  = false;
        bool right = false;
        bool front = false;
        bool back  = false;
        bool up    = false;
        bool down  = false;

        float relativeCursorX = 0;
        float relativeCursorY = 0;
    };

    DefaultCamera();

    void SetPosition(const Vec3 &position) noexcept;

    void SetDirection(float verticalAngle, float horizontalAngle) noexcept;

    void SetFOVy(float degree) noexcept;

    void SetWOverH(float wOverH) noexcept;

    void SetClipDistance(float nearPlane, float farPlane) noexcept;

    void SetMoveSpeed(float unitPerSecond) noexcept;

    void SetViewSpeed(float radPerPixel) noexcept;

    void Update(const Input &input, float deltaT) noexcept;

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
