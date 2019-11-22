#pragma once

#include <VRPG/Game/World/Block/BasicEffect/DiffuseSolidBlockEffect.h>
#include <VRPG/Game/World/Block/BlockDescription.h>

VRPG_GAME_BEGIN

class DiffuseSolidBoxDescription : public BlockDescription
{
public:

    DiffuseSolidBoxDescription(
        std::string name,
        std::shared_ptr<const DiffuseSolidBlockEffect> effect, int textureIndexInEffect[],
        BlockBrightness emission);

    const char *GetName() const override;

    FaceVisibilityType GetFaceVisibility(Direction direction) const noexcept override;

    bool IsVisible() const noexcept override;

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

    std::shared_ptr<const DiffuseSolidBlockEffect> effect_;
    int textureIndexInEffect_[6];

    bool isLightSource_;
    BlockBrightness emission_;
};

VRPG_GAME_END
