#pragma once

#include <VRPG/Game/World/Block/BasicEffect/GrassLikeEffect.h>
#include <VRPG/Game/World/Block/BlockDescription.h>

VRPG_GAME_BEGIN

class GrassLikeDescription : public BlockDescription
{
public:

    GrassLikeDescription(
        std::string name,
        std::shared_ptr<const GrassLikeEffect> effect, int textureIndexInEffect[2]);

    const char *GetName() const override;

    FaceVisibilityType GetFaceVisibility(Direction direction) const noexcept override;

    bool IsReplacableByLiquid() const noexcept override;

    bool IsFullOpaque() const noexcept override;

    bool IsLightSource() const noexcept override;

    BlockBrightness LightAttenuation() const noexcept override;

    BlockBrightness InitialBrightness() const noexcept override;

    void AddBlockModel(
        ModelBuilderSet &modelBuilders,
        const Vec3i &blockPosition,
        const BlockNeighborhood blocks) const override;

    const BlockCollision *GetCollision() const noexcept override;

private:

    std::string name_;

    std::shared_ptr<const GrassLikeEffect> effect_;
    int textureIndexInEffect_[2];
};

VRPG_GAME_END
