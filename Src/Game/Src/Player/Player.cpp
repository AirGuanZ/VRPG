#include <VRPG/Game/Player/Player.h>

VRPG_GAME_BEGIN

Player::Player(const ChunkManager &chunkManager, const Vec3 &initPosition)
{
    state_            = State::Standing;
    onGround_         = true;
    position_         = initPosition;
    horizontalRadian_ = 0;
    chunkManager_     = &chunkManager;

    HandleMovement({}, 1);
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

void Player::HandleMovement(const UserInput &userInput, float dt)
{
    // TODO
}

VRPG_GAME_END
