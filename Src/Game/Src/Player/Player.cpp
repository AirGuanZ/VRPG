#include <VRPG/Game/Config/GlobalConfig.h>
#include <VRPG/Game/Player/Player.h>
#include <VRPG/Game/World/Block/BlockCollision.h>
#include <VRPG/Game/World/Chunk/ChunkManager.h>

VRPG_GAME_BEGIN

namespace
{
    /*
     * 在origin的dir方向上叠加一个长度为accV的分量
     * 并用maxV作为叠加得到的分量长度的上限
     * 若origin在dir方向上的长度原本就超过maxV，则不受影响
     * dir shall be normalized
     */
    Vec3 CombineAccel(const Vec3 &origin, const Vec3 &dir, float accV, float maxV)
    {
        //origin在dir方向上的分量长度
        float oriDirLen = dot(origin, dir);
        //新的dir方向上的分量长度
        float newDirLen = (std::max)(oriDirLen, (std::min)(oriDirLen + accV, maxV));
        //合成为最终结果
        return (origin - oriDirLen * dir) + newDirLen * dir;
    }

    /* 在origin的-dir方向上叠加一个长度为fric的分量
     * 并用minV作为叠加得到的分量长度的下限
     * 若分量长度原本就低于minV，则不受影响
     * dir shall be normalized
     */
    Vec3 CombineFriction(const Vec3 &origin, const Vec3 &dir, float fric, float minV)
    {
        float oriDirLen = dot(origin, dir);
        float newDirLen = (std::min)(oriDirLen, (std::max)(oriDirLen - fric, minV));
        return (origin - oriDirLen * dir) + newDirLen * dir;
    }

    Vec3 ComputeXZMoveDirection(const Player::UserInput &userInput, const Vec3 &playerDirection) noexcept
    {
        int front = static_cast<int>(userInput.frontPressed) - static_cast<int>(userInput.backPressed);
        int left  = static_cast<int>(userInput.leftPressed)  - static_cast<int>(userInput.rightPressed);
        if(!front && !left)
        {
            return Vec3(0);
        }

        Vec3 horMove = static_cast<float>(front) * Vec3(playerDirection.x, 0, playerDirection.z)
                     - static_cast<float>(left)  * Vec3(playerDirection.z, 0, -playerDirection.x);
        return horMove.normalize();
    }
    
    struct ResolveSolutionEnumerator
    {
        std::vector<BlockCollision::ResolveCollisionResult> *resolveSolutions = nullptr;
        float bestSolutionLenSqr = (std::numeric_limits<float>::max)();
        Vec3 bestSolution;

        Vec3 solution;
        void enumerate(size_t i)
        {
            if(i == resolveSolutions->size())
            {
                float sqr = solution.length_square();
                if(sqr < bestSolutionLenSqr)
                {
                    bestSolution = solution;
                    bestSolutionLenSqr = sqr;
                }
                return;
            }

            auto &thisSln = (*resolveSolutions)[i].axisAlignedOffset;

            // +x

            if(thisSln[0] > 0 && solution[0] >= 0)
            {
                float sol = solution[0];
                solution[0] = (std::max)(solution[0], thisSln[0]);
                enumerate(i + 1);
                solution[0] = sol;
            }

            // -x

            if(thisSln[0] < 0 && solution[0] <= 0)
            {
                float sol = solution[0];
                solution[0] = (std::min)(solution[0], thisSln[0]);
                enumerate(i + 1);
                solution[0] = sol;
            }

            // +y

            if(thisSln[1] > 0 && solution[1] >= 0)
            {
                float sol = solution[1];
                solution[1] = (std::max)(solution[1], thisSln[1]);
                enumerate(i + 1);
                solution[1] = sol;
            }

            // -y

            if(thisSln[1] < 0 && solution[1] <= 0)
            {
                float sol = solution[1];
                solution[1] = (std::min)(solution[1], thisSln[1]);
                enumerate(i + 1);
                solution[1] = sol;
            }

            // +z

            if(thisSln[2] > 0 && solution[2] >= 0)
            {
                float sol = solution[2];
                solution[2] = (std::max)(solution[2], thisSln[2]);
                enumerate(i + 1);
                solution[2] = sol;
            }

            // -z

            if(thisSln[2] < 0 && solution[2] <= 0)
            {
                float sol = solution[2];
                solution[2] = (std::min)(solution[2], thisSln[2]);
                enumerate(i + 1);
                solution[2] = sol;
            }
        }
    };
}

bool Player::PlayerParams::IsValid() const noexcept
{
#define NON_NEG(X) do { if(X < 0) return false; } while(false)

    NON_NEG(runningAccel);
    NON_NEG(walkingAccel);
    NON_NEG(floatingAccel);

    NON_NEG(runningMaxSpeed);
    NON_NEG(walkingMaxSpeed);
    NON_NEG(floatingMaxSpeed);

    NON_NEG(standingFrictionAccel);
    NON_NEG(runningFrictionAccel);
    NON_NEG(walkingFrictionAccel);
    NON_NEG(floatingFrictionAccel);

    NON_NEG(jumpingInitVelocity);
    NON_NEG(gravityAccel);
    NON_NEG(gravityMaxSpeed);

    NON_NEG(flyingAccel);
    NON_NEG(flyingFricAccel);

    NON_NEG(flyingMaxSpeed);
    NON_NEG(fastFlyingMaxSpeed);

    NON_NEG(flyingVertAccel);
    NON_NEG(flyingVertFricAccel);
    NON_NEG(flyingVertMaxSpeed);

    NON_NEG(cameraMoveXSpeed);
    NON_NEG(cameraMoveYSpeed);

    NON_NEG(cameraDownReOffset);
    NON_NEG(cameraUpReOffset);

    NON_NEG(collisionRadius);
    NON_NEG(collisionHeight);

    return true;

#undef NON_NEG
}

Player::Player(
    const PlayerParams &params, ChunkManager &chunkManager,
    const Vec3 &initPosition, const DefaultCamera &camera)
{
    if(!params.IsValid())
    {
        throw VRPGGameException("invalid player params");
    }

    params_       = params;
    chunkManager_ = &chunkManager;

    firstPerson_ = false;
    camera_      = camera;

    state_            = State::Standing;
    onGround_         = true;
    position_         = initPosition;

    cameraVerticalRadian_   = 0;
    cameraHorizontalRadian_ = 0;

    destPlayerHorizontalRadian_ = cameraHorizontalRadian_;
    playerHorizontalRadian_     = destPlayerHorizontalRadian_;

    enableRunning_   = false;
    enableCollision_ = true;

    cursorXHistory_ = ScalarHistory(4);
    cursorYHistory_ = ScalarHistory(4);

    playerEffect_ = CreateDiffuseSolidMeshEffect();
    playerMesh_ = DiffuseSolidMesh::LoadFromConfig(playerEffect_, GLOBAL_CONFIG.ASSET_PATH["Player"]["Mesh"]);
    playerMesh_->SetCurrentAnimation("Walking");
    playerMesh_->EnableAnimationLoop(true);
}

Vec3 Player::GetPosition() const noexcept
{
    return position_;
}

Vec3 Player::GetVelocity() const noexcept
{
    return velocity_;
}

Vec3 Player::GetCameraDirection() const noexcept
{
    return Vec3(
        std::cos(cameraHorizontalRadian_),
        0,
        std::sin(cameraHorizontalRadian_));
}

Vec3 Player::GetPlayerDirection() const noexcept
{
    return Vec3(
        std::cos(playerHorizontalRadian_),
        0,
        std::sin(playerHorizontalRadian_));
}

Collision::AACylinder Player::GetCollision() const noexcept
{
    return { position_, params_.collisionRadius, params_.collisionHeight };
}

bool Player::IsOnGround() const noexcept
{
    return onGround_;
}

const Camera &Player::GetCamera() const noexcept
{
    return camera_;
}

void Player::SetCameraWOverH(float wOverH) noexcept
{
    camera_.SetWOverH(wOverH);
}

void Player::Update(const UserInput &userInput, float dt)
{
    if(userInput.switchCollisionDown)
    {
        enableCollision_ = !enableCollision_;
    }

    if(userInput.switchFirstPerson)
    {
        firstPerson_ = !firstPerson_;
    }

    // 更新摄像机方向和角色方向

    UpdateDirection(userInput, dt);

    // 执行状态转移

    UpdateState(userInput, dt);

    // 根据速度计算新的位置，并进行碰撞检测与恢复
    // 在速度过大时，将dt拆成一小段一小段的，防止单帧位移过大导致碰撞检测失效

    float moveDt = dt;
    bool onGround = false;
    while(moveDt > 0)
    {
        float moveLen = velocity_.length() * moveDt;
        if(moveLen > 0.4f)
        {
            float ddt = moveDt * 0.4f / moveLen;
            UpdatePosition(ddt);
            onGround |= onGround_;
            moveDt = (std::max)(0.0f, moveDt - ddt);
        }
        else
        {
            UpdatePosition(moveDt);
            onGround |= onGround_;
            break;
        }
    }
    onGround_ = onGround;

    // 更新player mesh状态

    playerMesh_->SetCurrentAnimationTime(playerMesh_->GetCurrentAnimationTime() + 0.7f);
    playerMesh_->UpdateBoneTransform();
    playerMesh_->SetWorldTransform(
        Trans4::rotate_y(-playerHorizontalRadian_ - agz::math::PI_f / 2)
      * Trans4::translate(position_));

    // 更新摄像机位置

    if(firstPerson_)
    {
        camera_.SetPosition(position_ + Vec3(0, 1.6f, 0));
    }
    else
    {
        camera_.SetPosition(position_ + Vec3(0, 1.8f, 0) - camera_.GetDirection() * 4.0f);
    }
}

void Player::RenderShadow(const ShadowRenderParams &params) const
{
    playerEffect_->SetShadowRenderParams(params);
    playerEffect_->StartShadow();
    playerMesh_->RenderShadow(params);
    playerEffect_->EndShadow();
}

void Player::RenderForward(const ForwardRenderParams &params) const
{
    if(!firstPerson_)
    {
        playerMesh_->SetBrightness(Vec4(0, 0, 0, 1));
        playerEffect_->SetForwardRenderParams(params);
        playerEffect_->StartForward();
        playerMesh_->RenderForward(params);
        playerEffect_->EndForward();
    }
}

void Player::UpdateDirection(const UserInput &userInput, float dt)
{
    // 更新摄像机方向

    constexpr float PI = agz::math::PI_f;
    constexpr float PI_2 = 2 * PI;

    cursorXHistory_.Update(userInput.relativeCursorX);
    cursorYHistory_.Update(userInput.relativeCursorY);

    cameraVerticalRadian_   -= params_.cameraMoveYSpeed * cursorYHistory_.MeanValue();
    cameraHorizontalRadian_ -= params_.cameraMoveXSpeed * cursorXHistory_.MeanValue();

    cameraVerticalRadian_ = agz::math::clamp(
        cameraVerticalRadian_, -PI / 2 + params_.cameraDownReOffset, PI / 2 - params_.cameraUpReOffset);

    cameraHorizontalRadian_ = std::fmod(cameraHorizontalRadian_, PI_2);

    camera_.SetDirection(cameraVerticalRadian_, cameraHorizontalRadian_);

    // 更新角色目标方向

    if(HasMoving(userInput))
    {
        auto horMove = ComputeXZMoveDirection(userInput, camera_.GetDirection());
        destPlayerHorizontalRadian_ = std::atan2(horMove.z, horMove.x);
    }

    // 让角色方向往目标方向靠拢

    playerHorizontalRadian_     = std::fmod(playerHorizontalRadian_,     PI_2);
    destPlayerHorizontalRadian_ = std::fmod(destPlayerHorizontalRadian_, PI_2);

    if(firstPerson_)
    {
        playerHorizontalRadian_ = destPlayerHorizontalRadian_;
    }
    else
    {
        // 让player radian减小到dest radian所需要减小的弧度大小

        float playerMinusDest = playerHorizontalRadian_ - destPlayerHorizontalRadian_;
        if(playerMinusDest < 0)
        {
            playerMinusDest += PI_2;
        }

        // 让player radian增加到dest radian所需要增加的弧度大小

        float destMinusPlayer = destPlayerHorizontalRadian_ - playerHorizontalRadian_;
        if(destMinusPlayer < 0)
        {
            destMinusPlayer += PI_2;
        }

        // 哪个更小取哪个

        if(playerMinusDest < destMinusPlayer)
        {
            if(playerMinusDest > 0.75f * PI)
            {
                playerHorizontalRadian_ = destPlayerHorizontalRadian_;
            }
            else
            {
                // 减小一点player radian
                playerHorizontalRadian_ -= (std::min)(dt * 6, playerMinusDest);
            }
        }
        else
        {
            if(destMinusPlayer > 0.75f * PI)
            {
                playerHorizontalRadian_ = destPlayerHorizontalRadian_;
            }
            else
            {
                // 增加一点player radian
                playerHorizontalRadian_ += (std::min)(dt * 6, destMinusPlayer);
            }
        }
    }
}

void Player::UpdateState(const UserInput &userInput, float dt)
{
    State newState = State::Standing;
    switch(state_)
    {
    case State::Standing:   newState = TransState_Standing    (userInput); break;
    case State::Walking:    newState = TransState_Walking     (userInput); break;
    case State::Running:    newState = TransState_Running     (userInput); break;
    case State::Floating:   newState = TransState_Floating    (userInput); break;
    case State::Flying:     newState = TransState_Flying      (userInput); break;
    case State::FastFlying: newState = TransState_FastFlying(userInput); break;
    }

    if(newState != state_)
    {
        switch(newState)
        {
        case State::Standing:   InitState_Standing(userInput); break;
        case State::Walking:    InitState_Walking (userInput); break;
        case State::Running:    InitState_Running (userInput); break;
        case State::Floating:   InitState_Floating(userInput); break;
        case State::Flying:     InitState_Flying  (userInput); break;
        case State::FastFlying: InitState_FastFlying(userInput); break;
        }
        state_ = newState;
    }

    switch(state_)
    {
    case State::Standing:   ApplyState_Standing(userInput, dt); break;
    case State::Walking:    ApplyState_Walking (userInput, dt); break;
    case State::Running:    ApplyState_Running (userInput, dt); break;
    case State::Floating:   ApplyState_Floating(userInput, dt); break;
    case State::Flying:     ApplyState_Flying(userInput, dt);   break;
    case State::FastFlying: ApplyState_FastFlying(userInput, dt); break;
    }
}

void Player::UpdatePosition(float dt)
{
    Vec3 deltaPosition = dt * velocity_;
    Vec3 newPosition = position_ + deltaPosition;

    if(!enableCollision_)
    {
        position_ = newPosition;
        return;
    }

    Vec3 lowf  = newPosition - Vec3(params_.collisionRadius, 0, params_.collisionRadius)
                             - Vec3(0.2f);
    Vec3 highf = newPosition + Vec3(params_.collisionRadius,
                                    params_.collisionHeight,
                                    params_.collisionRadius)
                             + Vec3(0.2f);
    Vec3i lowi  = lowf .map([](float f) { return static_cast<int>(std::floor(f)); });
    Vec3i highi = highf.map([](float f) { return static_cast<int>(std::ceil(f)); });

    static std::vector<BlockCollision::ResolveCollisionResult> resolveSolutions;
    resolveSolutions.clear();

    for(int x = lowi.x; x <= highi.x; ++x)
    {
        for(int z = lowi.z; z <= highi.z; ++z)
        {
            for(int y = lowi.y; y <= highi.y; ++y)
            {
                auto collision   = chunkManager_->GetBlockDesc({ x, y, z })->GetCollision();
                auto orientation = chunkManager_->GetBlockOrientation({ x, y, z });

                Vec3 localPosition = {
                    position_.x - static_cast<float>(x),
                    position_.y - static_cast<float>(y),
                    position_.z - static_cast<float>(z)
                };
                Vec3 localNewPosition = {
                    newPosition.x - static_cast<float>(x),
                    newPosition.y - static_cast<float>(y),
                    newPosition.z - static_cast<float>(z),
                };

                Collision::AACylinder cylinder{
                    localPosition,
                    params_.collisionRadius,
                    params_.collisionHeight
                };

                BlockCollision::ResolveCollisionResult result;
                if(collision->ResolveCollisionWith(orientation, cylinder, localNewPosition, &result))
                {
                    resolveSolutions.push_back(result);
                }
            }
        }
    }

    onGround_ = false;
    if(resolveSolutions.empty())
    {
        position_ = newPosition;
        return;
    }

    ResolveSolutionEnumerator enumerator;
    enumerator.resolveSolutions = &resolveSolutions;
    enumerator.enumerate(0);

    if(enumerator.bestSolutionLenSqr == (std::numeric_limits<float>::max)())
    {
        velocity_ = Vec3();
        return;
    }

    if(enumerator.bestSolution.x != 0)
    {
        velocity_.x = 0;
    }

    if(enumerator.bestSolution.y != 0)
    {
        velocity_.y = 0;
    }

    if(enumerator.bestSolution.z != 0)
    {
        velocity_.z = 0;
    }

    if(enumerator.bestSolution.y > 0)
    {
        onGround_ = true;
    }

    Vec3 resolvedNewPosition       = newPosition + enumerator.bestSolution;
    Vec3 resolvedDeltaPosition     = resolvedNewPosition - position_;
    float resolvedDeltaPositionLen = resolvedDeltaPosition.length();
    if(resolvedDeltaPositionLen < 1e-2f)
    {
        return;
    }

    Vec3 resolvedDeltaDir         = resolvedDeltaPosition / resolvedDeltaPositionLen;
    float maxResolvedDeltaLen     = (std::max)(0.0f, dot(deltaPosition, resolvedDeltaDir));
    float clampedResolvedDeltaLen = (std::min)(resolvedDeltaPositionLen, maxResolvedDeltaLen);

    position_ += clampedResolvedDeltaLen * resolvedDeltaDir;
}

bool Player::HasMoving(const UserInput &userInput) noexcept
{
    return (userInput.leftPressed ^ userInput.rightPressed) ||
           (userInput.backPressed ^ userInput.frontPressed);
}

void Player::InitState_Standing(const UserInput &userInput)
{
    enableRunning_ = false;
    velocity_.y = 0;
}

void Player::InitState_Walking(const UserInput &userInput)
{
    velocity_.y = 0;
}

void Player::InitState_Running(const UserInput &userInput)
{
    velocity_.y = 0;
}

void Player::InitState_Floating(const UserInput &userInput)
{
    if(onGround_ && userInput.jumpPressed)
    {
        velocity_.y = params_.jumpingInitVelocity;
    }
}

void Player::InitState_Flying(const UserInput &userInput)
{
    enableRunning_ = false;
}

void Player::InitState_FastFlying(const UserInput &userInput)
{
    enableRunning_ = false;
}

Player::State Player::TransState_Standing(const UserInput &userInput)
{
    if(userInput.flyDown || !enableCollision_)
    {
        return enableRunning_ ? State::FastFlying : State::Flying;
    }

    if(!onGround_ || userInput.jumpPressed)
    {
        return State::Floating;
    }

    if(HasMoving(userInput))
    {
        return enableRunning_ ? State::Running : State::Walking;
    }

    return State::Standing;
}

Player::State Player::TransState_Walking(const UserInput &userInput)
{
    if(userInput.runDown)
    {
        enableRunning_ = !enableRunning_;
    }

    if(userInput.flyDown || !enableCollision_)
    {
        return enableRunning_ ? State::FastFlying : State::Flying;
    }

    if(!onGround_ || userInput.jumpPressed)
    {
        return State::Floating;
    }

    if(HasMoving(userInput))
    {
        return enableRunning_ ? State::Running : State::Walking;
    }

    return State::Standing;
}

Player::State Player::TransState_Running(const UserInput &userInput)
{
    if(userInput.runDown)
    {
        enableRunning_ = !enableRunning_;
    }

    if(userInput.flyDown || !enableCollision_)
    {
        return enableRunning_ ? State::FastFlying : State::Flying;
    }

    if(!onGround_ || userInput.jumpPressed)
    {
        return State::Floating;
    }

    if(HasMoving(userInput))
    {
        return enableRunning_ ? State::Running : State::Walking;
    }

    return State::Standing;
}

Player::State Player::TransState_Floating(const UserInput &userInput)
{
    if(userInput.flyDown || !enableCollision_)
    {
        return enableRunning_ ? State::FastFlying : State::Flying;
    }

    if(!onGround_)
    {
        return State::Floating;
    }

    if(HasMoving(userInput))
    {
        return enableRunning_ ? State::Running : State::Walking;
    }

    return State::Standing;
}

Player::State Player::TransState_Flying(const UserInput &userInput)
{
    if(userInput.flyDown)
    {
        return State::Floating;
    }

    if(userInput.runDown)
    {
        return State::FastFlying;
    }

    return State::Flying;
}

Player::State Player::TransState_FastFlying(const UserInput &userInput)
{
    if(userInput.flyDown)
    {
        return State::Floating;
    }

    if(userInput.runDown)
    {
        return State::Flying;
    }

    return State::FastFlying;
}

void Player::ApplyState_Standing(const UserInput &userInput, float dt)
{
    velocity_ = CombineAccel(velocity_, Vec3(0, -1, 0), dt * params_.gravityAccel, params_.gravityMaxSpeed);

    Vec3 fricDir = Vec3(velocity_.x, 0, velocity_.z);
    if(fricDir.length() > 1e-5f)
    {
        fricDir = fricDir.normalize();
        velocity_ = CombineFriction(velocity_, fricDir, dt * params_.standingFrictionAccel, 0);
    }
}

void Player::ApplyState_Walking(const UserInput &userInput, float dt)
{
    velocity_ = CombineAccel(velocity_, Vec3(0, -1, 0), dt * params_.gravityAccel, params_.gravityMaxSpeed);

    Vec3 fricDir = Vec3(velocity_.x, 0, velocity_.z);
    if(fricDir.length() > 1e-5f)
    {
        fricDir = fricDir.normalize();
        velocity_ = CombineFriction(velocity_, fricDir, dt * params_.walkingFrictionAccel, 0);
    }

    if(HasMoving(userInput))
    {
        Vec3 horMove = GetPlayerDirection();//ComputeXZMoveDirection(userInput, GetPlayerDirection());
        velocity_ = CombineAccel(velocity_, horMove, dt * params_.walkingAccel, params_.walkingMaxSpeed);
    }
}

void Player::ApplyState_Running(const UserInput &userInput, float dt)
{
    velocity_ = CombineAccel(velocity_, Vec3(0, -1, 0), dt * params_.gravityAccel, params_.gravityMaxSpeed);

    Vec3 fricDir = Vec3(velocity_.x, 0, velocity_.z);
    if(fricDir.length() > 1e-5f)
    {
        fricDir = fricDir.normalize();
        velocity_ = CombineFriction(velocity_, fricDir, dt * params_.runningFrictionAccel, 0);
    }

    if(HasMoving(userInput))
    {
        Vec3 horMove = GetPlayerDirection();//ComputeXZMoveDirection(userInput, GetPlayerDirection());
        velocity_ = CombineAccel(velocity_, horMove, dt * params_.runningAccel, params_.runningMaxSpeed);
    }
}

void Player::ApplyState_Floating(const UserInput &userInput, float dt)
{
    velocity_ = CombineAccel(velocity_, Vec3(0, -1, 0), dt * params_.gravityAccel, params_.gravityMaxSpeed);
    
    Vec3 fricDir = Vec3(velocity_.x, 0, velocity_.z);
    if(fricDir.length() > 1e-5f)
    {
        fricDir = fricDir.normalize();
        velocity_ = CombineFriction(velocity_, fricDir, dt * params_.floatingFrictionAccel, 0);
    }

    float floatingMaxSpeed = params_.floatingMaxSpeed;
    if(enableRunning_)
    {
        floatingMaxSpeed *= params_.runningMaxSpeed / params_.walkingMaxSpeed;
    }

    if(HasMoving(userInput))
    {
        Vec3 horMove = GetPlayerDirection();//ComputeXZMoveDirection(userInput, GetPlayerDirection());
        velocity_ = CombineAccel(velocity_, horMove, dt * params_.floatingAccel, floatingMaxSpeed);
    }
}

void Player::ApplyState_Flying(const UserInput &userInput, float dt)
{
    Vec3 horiFricDir = Vec3(velocity_.x, 0, velocity_.z);
    if(horiFricDir.length() > 1e-5f)
    {
        horiFricDir = horiFricDir.normalize();
        velocity_ = CombineFriction(velocity_, horiFricDir, dt * params_.flyingFricAccel, 0);
    }

    if(std::abs(velocity_.y) > 1e-5f)
    {
        Vec3 vertFricDir = Vec3(0, velocity_.y > 0 ? 1.0f : -1.0f, 0);
        velocity_ = CombineFriction(velocity_, vertFricDir, dt * params_.flyingVertFricAccel, 0);
    }

    if(HasMoving(userInput))
    {
        Vec3 horiMove = GetPlayerDirection();//ComputeXZMoveDirection(userInput, GetPlayerDirection());
        velocity_ = CombineAccel(velocity_, horiMove, dt * params_.flyingAccel, params_.flyingMaxSpeed);
    }

    if(userInput.upPressed ^ userInput.downPressed)
    {
        Vec3 vertMove;
        vertMove.y = static_cast<float>(static_cast<int>(userInput.upPressed)
                                      - static_cast<int>(userInput.downPressed));
        velocity_ = CombineAccel(velocity_, vertMove, dt * params_.flyingVertAccel, params_.flyingVertMaxSpeed);
    }
}

void Player::ApplyState_FastFlying(const UserInput &userInput, float dt)
{
    Vec3 horiFricDir = Vec3(velocity_.x, 0, velocity_.z);
    if(horiFricDir.length() > 1e-5f)
    {
        horiFricDir = horiFricDir.normalize();
        velocity_ = CombineFriction(velocity_, horiFricDir, dt * params_.flyingFricAccel, 0);
    }

    if(std::abs(velocity_.y) > 1e-5f)
    {
        Vec3 vertFricDir = Vec3(0, velocity_.y > 0 ? 1.0f : -1.0f, 0);
        velocity_ = CombineFriction(velocity_, vertFricDir, dt * params_.flyingVertFricAccel, 0);
    }

    if(HasMoving(userInput))
    {
        Vec3 horiMove = GetPlayerDirection();//ComputeXZMoveDirection(userInput, GetPlayerDirection());
        velocity_ = CombineAccel(velocity_, horiMove, dt * params_.flyingAccel, params_.fastFlyingMaxSpeed);
    }

    if(userInput.upPressed ^ userInput.downPressed)
    {
        Vec3 vertMove;
        vertMove.y = static_cast<float>(static_cast<int>(userInput.upPressed)
            - static_cast<int>(userInput.downPressed));
        velocity_ = CombineAccel(velocity_, vertMove, dt * params_.flyingVertAccel, params_.flyingVertMaxSpeed);
    }
}

VRPG_GAME_END
