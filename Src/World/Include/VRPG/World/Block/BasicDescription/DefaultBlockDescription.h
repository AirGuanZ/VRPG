#pragma once

#include <VRPG/World/Block/BlockDescription.h>

VRPG_WORLD_BEGIN

class DefaultBlockEffect;

class DefaultBlockDescription : public BlockDescription
{
    const DefaultBlockEffect *effect_;

public:

    explicit DefaultBlockDescription(const DefaultBlockEffect *effect) noexcept;

    const char *GetName() const override { return "default"; }

    bool IsFullOpaque(Direction direction) const noexcept override;

    bool IsVisible() const noexcept override;

    void AddBlockModel(
        PartialSectionModelBuilderSet &modelBuilders,
        const Vec3i &blockPosition,
        const BlockDescription *neighborBlocks[3][3][3],
        const BlockBrightness neighborBrightness[3][3][3],
        const BlockOrientation neighborOrientations[3][3][3]) const override;

    bool IsLightSource() const noexcept override;

    BlockBrightness LightAttenuation() const noexcept override;

    BlockBrightness InitialBrightness() const noexcept override;
};

VRPG_WORLD_END
