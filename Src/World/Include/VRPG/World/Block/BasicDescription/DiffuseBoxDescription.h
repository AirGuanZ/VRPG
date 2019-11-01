#pragma once

#include <VRPG/World/Block/BasicEffect/DiffuseBlockEffect.h>
#include <VRPG/World/Block/BlockDescription.h>

VRPG_WORLD_BEGIN

class DiffuseBoxDescription : public BlockDescription
{
public:

    DiffuseBoxDescription(
        std::string name,
        std::shared_ptr<const DiffuseBlockEffect> effect, int textureIndexInEffect[],
        BlockBrightness emission);

    const char *GetName() const override;

    bool IsFullOpaque(Direction direction) const noexcept override;

    bool IsVisible() const noexcept override;

    bool IsLightSource() const noexcept override;

    BlockBrightness LightAttenuation() const noexcept override;

    BlockBrightness InitialBrightness() const noexcept override;

    void AddBlockModel(
        PartialSectionModelBuilderSet &modelBuilders,
        const Vec3i &blockPosition,
        const BlockDescription *neighborBlocks[3][3][3],
        const BlockBrightness neighborBrightness[3][3][3],
        const BlockOrientation neighborOrientations[3][3][3]) const override;

private:

    std::string name_;

    std::shared_ptr<const DiffuseBlockEffect> effect_;
    int textureIndexInEffect_[6];

    bool isLightSource_;
    BlockBrightness emission_;
};

VRPG_WORLD_END
