﻿#pragma once

#include <VRPG/Game/World/Block/BasicEffect/TransparentBlockEffect.h>
#include <VRPG/Game/World/Block/BlockDescription.h>

VRPG_GAME_BEGIN

class TransparentBoxDescription : public BlockDescription
{
public:

    TransparentBoxDescription(
        std::string name,
        std::shared_ptr<TransparentBlockEffect> effect, int textureIndexInEffect[],
        BlockBrightness attenuation);

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

    std::shared_ptr<const TransparentBlockEffect> effect_;
    int textureIndexInEffect_[6];

    BlockBrightness attenuation_;
};

VRPG_GAME_END
