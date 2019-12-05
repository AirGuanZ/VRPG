#include <VRPG/Game/Misc/RayBoxIntersect.h>
#include <VRPG/Game/World/Block/BasicCollision/BoxCollision.h>

VRPG_GAME_BEGIN

constexpr float EPS = 1e-3f;

BoxBlockCollision::BoxBlockCollision(bool enableCollision)
    : enableCollision_(enableCollision)
{
    
}

bool BoxBlockCollision::HasCollisionWith(BlockOrientation blockOrientation, const Collision::AACylinder &cylinder) const noexcept
{
    if(!enableCollision_)
    {
        return false;
    }

    if(cylinder.lowCentre.y >= 1 || cylinder.lowCentre.y + cylinder.height <= 0)
    {
        return false;
    }

    Vec2 p = cylinder.lowCentre.xz();
    Vec2 o = p.clamp(0, 1);
    return (p - o).length_square() < cylinder.radius * cylinder.radius;
}

bool BoxBlockCollision::ResolveCollisionWith(
    BlockOrientation blockOrientation,
    const Collision::AACylinder &cylinder, const Vec3 &newLowCentre, ResolveCollisionResult *result) const noexcept
{
    assert(result);

    if(!enableCollision_)
    {
        return false;
    }

    // +/-y

    float lowY = newLowCentre.y, highY = lowY + cylinder.height;
    if(lowY >= 1 || highY <= 0)
    {
        return false;
    }
    else
    {
        if(cylinder.lowCentre.y < newLowCentre.y)
        {
            result->axisAlignedOffset[1] = -highY - EPS;
        }
        else
        {
            result->axisAlignedOffset[1] = 1 - lowY + EPS;
        }
    }

    Vec2 p = newLowCentre.xz();
    Vec2 o = p.clamp(0, 1);
    float r2 = cylinder.radius * cylinder.radius;
    if((p - o).length_square() >= r2)
    {
        return false;
    }

    // +/-x

    if(cylinder.lowCentre.x < p.x)
    {
        float deltaY = p.y - o.y;
        float newX = -std::sqrt(std::max(0.0f, r2 - deltaY * deltaY));
        result->axisAlignedOffset[0] = newX - p.x;
    }
    else
    {
        float deltaY = p.y - o.y;
        float newX = 1 + std::sqrt(std::max(0.0f, r2 - deltaY * deltaY));
        result->axisAlignedOffset[0] = newX - p.x;
    }

    // +/-z

    if(cylinder.lowCentre.z < p.y)
    {
        float deltaX = p.x - o.x;
        float newZ = -std::sqrt(std::max(0.0f, r2 - deltaX * deltaX));
        result->axisAlignedOffset[2] = newZ - p.y;
    }
    else
    {
        float deltaX = p.x - o.x;
        float newZ = 1 + std::sqrt(std::max(0.0f, r2 - deltaX * deltaX));
        result->axisAlignedOffset[2] = newZ - p.y;
    }

    return true;
}

bool BoxBlockCollision::IntersectWith(const Collision::Ray &ray, Direction *pickedFace) const noexcept
{
    return RayIntersectStdBox(ray.o, ray.d, ray.t0, ray.t1, pickedFace);
}

VRPG_GAME_END
