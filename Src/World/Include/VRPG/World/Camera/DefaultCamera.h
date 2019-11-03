#pragma once

#include <VRPG/World/Camera/Camera.h>
#include <VRPG/World/Utility/ScalarHistory.h>

VRPG_WORLD_BEGIN

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

    mutable bool isMatrixDirty_;
    mutable Mat4 viewProjectionMatrix_;

    void UpdateViewProjectionMatrix() const noexcept;

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

    Mat4 GetViewProjectionMatrix() const override;

    Vec3 GetPosition() const noexcept override;

    Vec3 GetDirection() const noexcept override;
};

VRPG_WORLD_END
