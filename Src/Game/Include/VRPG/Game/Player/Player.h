#pragma once

#include <VRPG/Game/Camera/DefaultCamera.h>
#include <VRPG/Game/Utility/ScalarHistory.h>

VRPG_GAME_BEGIN

class ChunkManager;

class Player : public agz::misc::uncopyable_t
{
public:

    using PlayerCamera = DefaultCamera;

    // 用于控制角色行动速度和碰撞的参数
    struct PlayerParams
    {
        // 重力模式移动参数

        float runningAccel  = 300.0f;         // 跑动自给水平加速度
        float walkingAccel  = 200.0f;         // 行走自给水平加速度
        float floatingAccel = 30.0f;          // 浮空自给水平加速度

        float runningMaxSpeed  = 8.5f;        // 跑动最大自给水平速度
        float walkingMaxSpeed  = 6.5f;        // 行走最大自给水平速度
        float floatingMaxSpeed = 6.5f;        // 跳跃最大自给水平速度

        float standingFrictionAccel  = 80.0f; // 站立外来阻力水平加速度
        float runningFrictionAccel   = 80.0f; // 跑动外来阻力水平加速度
        float walkingFrictionAccel   = 80.0f; // 行走外来阻力水平加速度
        float floatingFrictionAccel  = 10.0f; // 跳跃外来阻力水平加速度

        float jumpingInitVelocity = 10.0f;    // 跳跃初始速度
        float gravityAccel        = 30.0;     // 重力加速度
        float gravityMaxSpeed     = 120.0f;   // 最大下落速度

        // 飞行模式移动参数

        float flyingAccel       = 300.0f;
        float flyingFricAccel   = 30.0f;

        float flyingMaxSpeed     = 10.0f;
        float fastFlyingMaxSpeed = 15.0f;

        float flyingVertAccel     = 130.0f;
        float flyingVertFricAccel = 70.0f;
        float flyingVertMaxSpeed  = 10.0f;

        // 其他

        float cameraMoveXSpeed = 0.002f;       // 摄像机水平灵敏度
        float cameraMoveYSpeed = 0.002f;       // 摄像机垂直灵敏度

        float cameraDownReOffset = 0.02f;      // 摄像机最小垂直夹角偏移
        float cameraUpReOffset   = 0.02f;      // 摄像机最大垂直夹角偏移

        float collisionRadius = 0.25f;         // 角色碰撞半径（圆柱形碰撞体）
        float collisionHeight = 1.8f;          // 角色碰撞高度（圆柱形碰撞体）
    };

    // 用户用于操作角色行为的输入
    struct UserInput
    {
        bool leftPressed  = false;
        bool rightPressed = false;
        bool frontPressed = false;
        bool backPressed  = false;

        bool jumpPressed = false;
        bool runDown     = false;

        bool flyDown     = false;
        bool upPressed   = false;
        bool downPressed = false;

        bool switchCollisionDown = false;

        float relativeCursorX = 0;
        float relativeCursorY = 0;
    };

    Player(
        const PlayerParams &params, ChunkManager &chunkManager,
        const Vec3 &initPosition, const DefaultCamera &camera);

    Vec3 GetPosition() const noexcept;

    Vec3 GetVelocity() const noexcept;

    Vec3 GetDirection() const noexcept;

    Collision::AACylinder GetCollision() const noexcept;

    bool IsOnGround() const noexcept;

    const Camera &GetCamera() const noexcept;

    void SetCameraWOverH(float wOverH) noexcept;

    void HandleMovement(const UserInput &userInput, float dt);

private:

    void UpdateDirection(const UserInput &userInput);

    void UpdateState(const UserInput &userInput, float dt);

    void UpdatePosition(float dt);

    static bool HasMoving(const UserInput &userInput) noexcept;

    enum class State
    {
        Standing,
        Walking,
        Running,
        Floating,

        Flying,
        FastFlying
    };

    void InitState_Standing  (const UserInput &userInput);
    void InitState_Running   (const UserInput &userInput);
    void InitState_Walking   (const UserInput &userInput);
    void InitState_Floating  (const UserInput &userInput);
    void InitState_Flying    (const UserInput &userInput);
    void InitState_FastFlying(const UserInput &userInput);
    
    State TransState_Standing  (const UserInput &userInput);
    State TransState_Running   (const UserInput &userInput);
    State TransState_Walking   (const UserInput &userInput);
    State TransState_Floating  (const UserInput &userInput);
    State TransState_Flying    (const UserInput &userInput);
    State TransState_FastFlying(const UserInput &userInput);
    
    void ApplyState_Standing  (const UserInput &userInput, float dt);
    void ApplyState_Running   (const UserInput &userInput, float dt);
    void ApplyState_Walking   (const UserInput &userInput, float dt);
    void ApplyState_Floating  (const UserInput &userInput, float dt);
    void ApplyState_Flying    (const UserInput &userInput, float dt);
    void ApplyState_FastFlying(const UserInput &userInput, float dt);

    PlayerParams params_;
    ChunkManager *chunkManager_;

    State state_;
    bool onGround_;

    Vec3 position_;
    Vec3 velocity_;

    float verticalRadian_;
    float horizontalRadian_;
    ScalarHistory cursorXHistory_;
    ScalarHistory cursorYHistory_;

    bool enableRunning_;
    bool enableCollision_;

    PlayerCamera camera_;
};

VRPG_GAME_END
