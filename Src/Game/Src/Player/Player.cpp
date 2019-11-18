#include <VRPG/Game/Block/BlockCollision.h>
#include <VRPG/Game/Chunk/ChunkManager.h>
#include <VRPG/Game/Player/Player.h>

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

    /*
     * 将origin长度减少fric
     */
    Vec3 ApplyFriction(const Vec3 &origin, float fric)
    {
        float len = origin.length();
        if(len < 0.001f)
        {
            return Vec3();
        }
        Vec3 rt = origin / len;
        return (std::max)(0.0f, origin.length() - fric) * rt;
    }

    /*
     * 将origin在xz平面上分量的长度减少fric
     */
    Vec3 ApplyFrictionXZ(const Vec3 &ori, float fric)
    {
        Vec3 rtHor = ApplyFriction(Vec3(ori.x, 0.0f, ori.z), fric);
        return Vec3(rtHor.x, ori.y, rtHor.z);
    }

    Vec3 ComputeXZMoveDirection(const Player::UserInput &userInput, const Vec3 &cameraDirection) noexcept
    {
        int front = static_cast<int>(userInput.frontPressed) - static_cast<int>(userInput.backPressed);
        int left  = static_cast<int>(userInput.leftPressed)  - static_cast<int>(userInput.rightPressed);
        if(!front && !left)
            return Vec3(0);

        Vec3 horMove = static_cast<float>(front) * Vec3(cameraDirection.x, 0, cameraDirection.z)
                     - static_cast<float>(left)  * Vec3(cameraDirection.z, 0, -cameraDirection.x);
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

Player::Player(
    const PlayerParams &params, ChunkManager &chunkManager,
    const Vec3 &initPosition, const DefaultCamera &camera)
{
    params_       = params;
    chunkManager_ = &chunkManager;

    camera_ = camera;

    state_            = State::Standing;
    onGround_         = true;
    position_         = initPosition;
    verticalRadian_   = 0;
    horizontalRadian_ = 0;
}

Vec3 Player::GetPosition() const noexcept
{
    return position_;
}

Vec3 Player::GetVelocity() const noexcept
{
    return velocity_;
}

Vec3 Player::GetDirection() const noexcept
{
    return Vec3(
        std::cos(horizontalRadian_),
        0,
        std::sin(horizontalRadian_));
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

void Player::HandleMovement(const UserInput &userInput, float dt)
{
    UpdateDirection(userInput);
    UpdateState(userInput, dt);
    UpdatePosition(dt);
    camera_.SetPosition(position_ + Vec3(0, 1.6f, 0));
}

void Player::UpdateDirection(const UserInput &userInput)
{
    constexpr float PI = agz::math::PI_f;
    constexpr float PI_2 = 2 * PI;

    verticalRadian_   -= params_.cameraMoveYSpeed * userInput.relativeCursorY;
    horizontalRadian_ -= params_.cameraMoveXSpeed * userInput.relativeCursorX;

    verticalRadian_ = agz::math::clamp(
        verticalRadian_, -PI / 2 + params_.cameraDownReOffset, PI / 2 - params_.cameraUpReOffset);
    while(horizontalRadian_ > PI_2) horizontalRadian_ -= PI_2;
    while(horizontalRadian_ < 0)    horizontalRadian_ += PI_2;

    camera_.SetDirection(verticalRadian_, horizontalRadian_);
}

void Player::UpdateState(const UserInput &userInput, float dt)
{
    State newState = State::Standing;
    switch(state_)
    {
    case State::Standing: newState = TransState_Standing(userInput); break;
    case State::Walking:  newState = TransState_Walking (userInput); break;
    case State::Running:  newState = TransState_Running (userInput); break;
    case State::Floating: newState = TransState_Floating(userInput); break;
    }

    if(newState != state_)
    {
        switch(newState)
        {
        case State::Standing: InitState_Standing(userInput); break;
        case State::Walking:  InitState_Walking (userInput); break;
        case State::Running:  InitState_Running (userInput); break;
        case State::Floating: InitState_Floating(userInput); break;
        }
        state_ = newState;
    }

    switch(state_)
    {
    case State::Standing: ApplyState_Standing(userInput, dt); break;
    case State::Walking:  ApplyState_Walking (userInput, dt); break;
    case State::Running:  ApplyState_Running (userInput, dt); break;
    case State::Floating: ApplyState_Floating(userInput, dt); break;
    }
}

void Player::UpdatePosition(float dt)
{
    Vec3 deltaPosition = dt * velocity_;
    Vec3 newPosition = position_ + dt * velocity_;
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

    if(enumerator.bestSolution.y != 0)
    {
        velocity_.y = 0;
    }

    if(enumerator.bestSolution.y > 0)
    {
        onGround_ = true;
    }

    Vec3 resolvedNewPosition = newPosition + enumerator.bestSolution;
    Vec3 resolvedDeltaPosition = resolvedNewPosition - position_;
    float resolvedDeltaPositionLen = resolvedDeltaPosition.length();
    if(resolvedDeltaPositionLen < 1e-2f)
    {
        return;
    }

    Vec3 resolvedDeltaDir         = resolvedDeltaPosition / resolvedDeltaPositionLen;
    float maxResolvedDeltaLen     = (std::max)(0.0f, dot(deltaPosition, resolvedDeltaDir));
    float clampedResolvedDeltaLen = (std::min)(resolvedDeltaPositionLen, maxResolvedDeltaLen);

    position_ += clampedResolvedDeltaLen * resolvedDeltaDir;;
}

bool Player::HasMoving(const UserInput &userInput) noexcept
{
    return (userInput.leftPressed ^ userInput.rightPressed) ||
           (userInput.backPressed ^ userInput.frontPressed);
}

void Player::InitState_Standing(const UserInput &userInput)
{
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
        velocity_.y = params_.jumpingInitVelocity;
}

Player::State Player::TransState_Standing(const UserInput &userInput)
{
    if(!onGround_ || userInput.jumpPressed)
    {
        return State::Floating;
    }

    if(HasMoving(userInput))
    {
        return State::Walking;
    }

    return State::Standing;
}

Player::State Player::TransState_Walking(const UserInput &userInput)
{
    if(!onGround_ || userInput.jumpPressed)
    {
        return State::Floating;
    }

    if(HasMoving(userInput))
    {
        return State::Walking;
    }

    return State::Standing;
}

Player::State Player::TransState_Running(const UserInput &userInput)
{
    if(!onGround_ || userInput.jumpPressed)
    {
        return State::Floating;
    }

    if(HasMoving(userInput))
    {
        return State::Walking;
    }

    return State::Standing;
}

Player::State Player::TransState_Floating(const UserInput &userInput)
{
    if(!onGround_ || userInput.jumpPressed)
    {
        return State::Floating;
    }

    if(HasMoving(userInput))
    {
        return State::Walking;
    }

    return State::Standing;
}

void Player::ApplyState_Standing(const UserInput &userInput, float dt)
{
    velocity_ = CombineAccel(velocity_, Vec3(0, -1, 0), dt * params_.gravityAccel, params_.gravityMaxSpeed);
    velocity_ = ApplyFrictionXZ(velocity_, dt * params_.standingFrictionAccel);
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

    Vec3 horMove = ComputeXZMoveDirection(userInput, camera_.GetDirection());
    velocity_ = CombineAccel(velocity_, horMove, dt * params_.walkingAccel, params_.walkingMaxSpeed);
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

    Vec3 horMove = ComputeXZMoveDirection(userInput, camera_.GetDirection());
    velocity_ = CombineAccel(velocity_, horMove, dt * params_.runningAccel, params_.runningMaxSpeed);
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

    Vec3 horMove = ComputeXZMoveDirection(userInput, camera_.GetDirection());
    velocity_ = CombineAccel(velocity_, horMove, dt * params_.floatingAccel, params_.floatingMaxSpeed);
}

VRPG_GAME_END