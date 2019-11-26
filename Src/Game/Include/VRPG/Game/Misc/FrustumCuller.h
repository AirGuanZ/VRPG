#pragma once

#include <VRPG/Game/Common.h>

VRPG_GAME_BEGIN

struct CullingBoundingBox
{
    Vec3 low;
    Vec3 high;
};

class FrustumCuller
{
    Vec4 cullingFaces_[5];

    static bool IsNegativeForAllVertices(const Vec4 &f, const CullingBoundingBox &bbox) noexcept
    {
        return dot(f, { bbox.low.x,  bbox.low.y,  bbox.low.z,  1 }) < -0.01f &&
               dot(f, { bbox.low.x,  bbox.low.y,  bbox.high.z, 1 }) < -0.01f &&
               dot(f, { bbox.low.x,  bbox.high.y, bbox.low.z,  1 }) < -0.01f &&
               dot(f, { bbox.low.x,  bbox.high.y, bbox.high.z, 1 }) < -0.01f &&
               dot(f, { bbox.high.x, bbox.low.y,  bbox.low.z,  1 }) < -0.01f &&
               dot(f, { bbox.high.x, bbox.low.y,  bbox.high.z, 1 }) < -0.01f &&
               dot(f, { bbox.high.x, bbox.high.y, bbox.low.z,  1 }) < -0.01f &&
               dot(f, { bbox.high.x, bbox.high.y, bbox.high.z, 1 }) < -0.01f;
    }

public:

    FrustumCuller() = default;

    explicit FrustumCuller(const Mat4 &viewProjMatrix)
    {
        cullingFaces_[0] = viewProjMatrix.get_col(3) - viewProjMatrix.get_col(0);
        cullingFaces_[1] = viewProjMatrix.get_col(3) + viewProjMatrix.get_col(0);
        cullingFaces_[2] = viewProjMatrix.get_col(3) - viewProjMatrix.get_col(1);
        cullingFaces_[3] = viewProjMatrix.get_col(3) + viewProjMatrix.get_col(1);
        cullingFaces_[4] = viewProjMatrix.get_col(3);
    }

    bool IsVisible(const CullingBoundingBox &bbox) const noexcept
    {
        return !IsNegativeForAllVertices(cullingFaces_[0], bbox) &&
               !IsNegativeForAllVertices(cullingFaces_[1], bbox) &&
               !IsNegativeForAllVertices(cullingFaces_[2], bbox) &&
               !IsNegativeForAllVertices(cullingFaces_[3], bbox) &&
               !IsNegativeForAllVertices(cullingFaces_[4], bbox);
    }
};

VRPG_GAME_END
