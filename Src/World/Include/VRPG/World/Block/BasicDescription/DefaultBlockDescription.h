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
        agz::misc::span<std::unique_ptr<PartialSectionModelBuilder>> modelBuilders,
        const Vec3i &blockPosition,
        const BlockDescription *neighboringBlocks[3][3][3],
        BlockBrightness neighboringBrightness[3][3][3]) const override;

    bool IsLightSource() const noexcept override;

    BlockBrightness LightAttenuation() const noexcept override;

    BlockBrightness InitialBrightness() const noexcept override;
};

VRPG_WORLD_END
