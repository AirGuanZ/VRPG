#pragma once

#include <VRPG/Game/Camera/DefaultCamera.h>

VRPG_GAME_BEGIN

class ChunkManager;

class Player : public agz::misc::uncopyable_t
{
public:

    using PlayerCamera = DefaultCamera;

    // 用于控制角色行动速度和碰撞的参数
    struct PlayerParams
    {
        float runningAccel  = 0.0008f;         // 跑动自给水平加速度
        float walkingAccel  = 0.0007f;         // 行走自给水平加速度
        float floatingAccel = 0.0007f;         // 浮空自给水平加速度

        float runningMaxSpeed  = 0.0085f;      // 跑动最大自给水平速度
        float walkingMaxSpeed  = 0.0065f;      // 行走最大自给水平速度
        float floatingMaxSpeed = 0.0065f;      // 跳跃最大自给水平速度

        float standingFrictionAccel  = 0.0003f; // 站立外来阻力水平加速度
        float runningFrictionAccel   = 0.0003f; // 跑动外来阻力水平加速度
        float walkingFrictionAccel   = 0.0003f; // 行走外来阻力水平加速度
        float floatingFrictionAccel  = 0.0000f; // 跳跃外来阻力水平加速度

        float jumpingInitVelocity = 0.013f;    // 跳跃初始速度
        float gravityAccel        = 0.000038f; // 重力加速度
        float gravityMaxSpeed     = 0.03f;     // 最大下落速度

        float cameraMoveXSpeed   = 0.002f;     // 摄像机水平灵敏度
        float cameraMoveYSpeed   = 0.002f;     // 摄像机垂直灵敏度
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
        bool downPressed = false;
        bool runDown     = false;

        float relativeCursorX = 0;
        float relativeCursorY = 0;
    };

    Player(
        const PlayerParams &params, ChunkManager &chunkManager,
        const Vec3 &initPosition, const DefaultCamera &camera);

    Vec3 GetPosition() const noexcept;

    Vec3 GetVelocity() const noexcept;

    Vec3 GetDirection() const noexcept;

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
        Floating
    };

    void InitState_Standing(const UserInput &userInput);
    void InitState_Running (const UserInput &userInput);
    void InitState_Walking (const UserInput &userInput);
    void InitState_Floating(const UserInput &userInput);
    
    State TransState_Standing(const UserInput &userInput);
    State TransState_Running (const UserInput &userInput);
    State TransState_Walking (const UserInput &userInput);
    State TransState_Floating(const UserInput &userInput);
    
    void ApplyState_Standing(const UserInput &userInput, float dt);
    void ApplyState_Running (const UserInput &userInput, float dt);
    void ApplyState_Walking (const UserInput &userInput, float dt);
    void ApplyState_Floating(const UserInput &userInput, float dt);

    PlayerParams params_;
    ChunkManager *chunkManager_;

    State state_;
    bool onGround_;

    Vec3 position_;
    Vec3 velocity_;
    float verticalRadian_;
    float horizontalRadian_;

    PlayerCamera camera_;
};

VRPG_GAME_END
