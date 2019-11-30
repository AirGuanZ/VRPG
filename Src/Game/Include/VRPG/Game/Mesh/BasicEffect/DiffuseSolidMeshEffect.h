#pragma once

#include <VRPG/Game/Mesh/MeshEffect.h>

VRPG_GAME_BEGIN

class DiffuseSolidMeshEffect : public AnimatedMeshEffect
{
public:

    struct Vertex
    {
        Vec3 position;
        Vec3 normal;
        Vec2 texCoord;
    };

    // called by mesh in forward rendering

    virtual void BindForwardVSTransform(const Mat4 &world, const Mat4 &worldViewProj) const = 0;

    virtual void BindForwardPSDiffuseTexture(ID3D11ShaderResourceView *srv) const = 0;

    virtual void BindForwardPSBrightness(const Vec4 &brightness) const = 0;

    // called by mesh in shadow rendering

    virtual void BindShadowVSTransform(const Mat4 &worldViewProj) const = 0;
};

/**
 * @brief 根据global config中参数的不同创建对应的diffuse solid mesh effect
 */
std::shared_ptr<DiffuseSolidMeshEffect> CreateDiffuseSolidMeshEffect();

VRPG_GAME_END
