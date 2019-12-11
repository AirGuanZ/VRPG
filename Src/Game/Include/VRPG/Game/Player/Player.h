#pragma once

#include <VRPG/Game/Mesh/DiffuseSolidMesh.h>
#include <VRPG/Game/Misc/ScalarHistory.h>
#include <VRPG/Game/Physics/CollisionPrimitive.h>
#include <VRPG/Game/Player/Camera/DefaultCamera.h>

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

        float runningAccel           = -1; // 跑动自给水平加速度
        float walkingAccel           = -1; // 行走自给水平加速度
        float floatingAccel          = -1; // 浮空自给水平加速度

        float runningMaxSpeed        = -1; // 跑动最大自给水平速度
        float walkingMaxSpeed        = -1; // 行走最大自给水平速度
        float floatingMaxSpeed       = -1; // 跳跃最大自给水平速度

        float standingFrictionAccel  = -1; // 站立外来阻力水平加速度
        float runningFrictionAccel   = -1; // 跑动外来阻力水平加速度
        float walkingFrictionAccel   = -1; // 行走外来阻力水平加速度
        float floatingFrictionAccel  = -1; // 跳跃外来阻力水平加速度

        float jumpingInitVelocity    = -1; // 跳跃初始速度
        float gravityAccel           = -1; // 重力加速度
        float gravityMaxSpeed        = -1; // 最大下落速度

        // 飞行模式移动参数

        float flyingAccel            = -1; // 飞行自给水平加速度
        float flyingFricAccel        = -1; // 飞行外来阻力水平加速度

        float flyingMaxSpeed         = -1; // 飞行最大自给水平速度
        float fastFlyingMaxSpeed     = -1; // 快速飞行最大自给水品速度

        float flyingVertAccel        = -1; // 飞行自给垂直加速度
        float flyingVertFricAccel    = -1; // 飞行外来阻力垂直加速度
        float flyingVertMaxSpeed     = -1; // 飞行最大自给垂直速度

        // 其他

        float cameraMoveXSpeed       = -1; // 摄像机水平灵敏度
        float cameraMoveYSpeed       = -1; // 摄像机垂直灵敏度

        float cameraDownReOffset     = -1; // 摄像机最小垂直夹角偏移
        float cameraUpReOffset       = -1; // 摄像机最大垂直夹角偏移

        float collisionRadius        = -1; // 角色碰撞半径（圆柱形碰撞体）
        float collisionHeight        = -1; // 角色碰撞高度（圆柱形碰撞体）

        bool IsValid() const noexcept;
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
        bool switchFirstPerson   = false;

        float relativeCursorX = 0;
        float relativeCursorY = 0;
    };

    Player(
        const PlayerParams &params, ChunkManager &chunkManager,
        const Vec3 &initPosition, const DefaultCamera &camera);

    Vec3 GetPosition() const noexcept;

    Vec3 GetVelocity() const noexcept;

    Vec3 GetCameraDirection() const noexcept;

    Vec3 GetPlayerDirection() const noexcept;

    Collision::AACylinder GetCollision() const noexcept;

    bool IsOnGround() const noexcept;

    const Camera &GetCamera() const noexcept;

    void SetCameraWOverH(float wOverH) noexcept;

    void Update(const UserInput &userInput, float dt);

    void RenderShadow(const ShadowRenderParams &params) const;

    void RenderForward(const ForwardRenderParams &params) const;

private:

    void UpdateDirection(const UserInput &userInput, float dt);

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
    void InitState_Walking   (const UserInput &userInput);
    void InitState_Running   (const UserInput &userInput);
    void InitState_Floating  (const UserInput &userInput);
    void InitState_Flying    (const UserInput &userInput);
    void InitState_FastFlying(const UserInput &userInput);
    
    State TransState_Standing  (const UserInput &userInput);
    State TransState_Walking   (const UserInput &userInput);
    State TransState_Running   (const UserInput &userInput);
    State TransState_Floating  (const UserInput &userInput);
    State TransState_Flying    (const UserInput &userInput);
    State TransState_FastFlying(const UserInput &userInput);
    
    void ApplyState_Standing  (const UserInput &userInput, float dt);
    void ApplyState_Walking   (const UserInput &userInput, float dt);
    void ApplyState_Running   (const UserInput &userInput, float dt);
    void ApplyState_Floating  (const UserInput &userInput, float dt);
    void ApplyState_Flying    (const UserInput &userInput, float dt);
    void ApplyState_FastFlying(const UserInput &userInput, float dt);

    PlayerParams params_;
    ChunkManager *chunkManager_;

    State state_;
    bool onGround_;

    Vec3 position_;
    Vec3 velocity_;
    Vec3 lastVelocity_;

    float cameraVerticalRadian_;
    float cameraHorizontalRadian_;

    float playerHorizontalRadian_;
    float playerModelHorizontalRadian_;

    ScalarHistory cursorXHistory_;
    ScalarHistory cursorYHistory_;

    bool enableRunning_;
    bool enableCollision_;

    bool firstPerson_;
    PlayerCamera camera_;

    std::shared_ptr<DiffuseSolidMeshEffect> playerEffect_;
    std::unique_ptr<DiffuseSolidMesh>       playerMesh_;
};

VRPG_GAME_END
