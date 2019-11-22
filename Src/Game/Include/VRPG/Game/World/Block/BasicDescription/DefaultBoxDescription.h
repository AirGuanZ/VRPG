#pragma once

#include <VRPG/Game/World/Block/BlockDescription.h>

VRPG_GAME_BEGIN

class DefaultBlockEffect;

class DefaultBlockDescription : public BlockDescription
{
    std::shared_ptr<const DefaultBlockEffect> effect_;

public:

    explicit DefaultBlockDescription(std::shared_ptr<const DefaultBlockEffect> effect) noexcept;

    const char *GetName() const override { return "default"; }

    FaceVisibilityType GetFaceVisibility(Direction direction) const noexcept override;

    bool IsVisible() const noexcept override;

    bool IsFullOpaque() const noexcept override;

    bool IsLightSource() const noexcept override;

    BlockBrightness LightAttenuation() const noexcept override;

    BlockBrightness InitialBrightness() const noexcept override;

    void AddBlockModel(
        ModelBuilderSet &modelBuilders,
        const Vec3i &blockPosition,
        const BlockNeighborhood neighborhood) const override;

    const BlockCollision *GetCollision() const noexcept override;
};

VRPG_GAME_END
