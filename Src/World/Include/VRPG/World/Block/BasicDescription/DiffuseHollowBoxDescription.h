#pragma once

#include <VRPG/World/Block/BasicEffect/DiffuseHollowBlockEffect.h>
#include <VRPG/World/Block/BlockDescription.h>

VRPG_WORLD_BEGIN

class DiffuseHollowBoxDescription : public BlockDescription
{
public:

    DiffuseHollowBoxDescription(
        std::string name,
        std::shared_ptr<const DiffuseHollowBlockEffect> effect, int textureIndexInEffect[],
        BlockBrightness emission,
        BlockBrightness attenuation);

    const char *GetName() const override;

    FaceVisibilityProperty GetFaceVisibilityProperty(Direction direction) const noexcept override;

    bool IsVisible() const noexcept override;

    bool IsFullOpaque() const noexcept override;

    bool IsLightSource() const noexcept override;

    BlockBrightness LightAttenuation() const noexcept override;

    BlockBrightness InitialBrightness() const noexcept override;

    void AddBlockModel(
        PartialSectionModelBuilderSet &modelBuilders,
        const Vec3i &blockPosition,
        const BlockNeighborhood blocks) const override;

private:

    std::string name_;

    std::shared_ptr<const DiffuseHollowBlockEffect> effect_;
    int textureIndexInEffect_[6];

    bool isLightSource_;
    BlockBrightness emission_;
    BlockBrightness attenuation_;
};

VRPG_WORLD_END
