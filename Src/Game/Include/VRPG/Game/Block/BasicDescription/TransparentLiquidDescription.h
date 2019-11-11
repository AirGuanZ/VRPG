#pragma once

#include <VRPG/Game/Block/BasicEffect/DefaultBlockEffect.h>
#include <VRPG/Game/Block/BlockDescription.h>

VRPG_GAME_BEGIN

class TransparentLiquidDescription : public BlockDescription
{
public:

    TransparentLiquidDescription(
        std::string name, LiquidDescription liquid, BlockBrightness attenuation);

    const char *GetName() const override;

    FaceVisibilityProperty GetFaceVisibilityProperty(Direction direction) const noexcept override;

    bool IsVisible() const noexcept override;

    bool IsReplacable() const noexcept override;

    bool IsFullOpaque() const noexcept override;

    bool IsLightSource() const noexcept override;

    BlockBrightness LightAttenuation() const noexcept override;

    BlockBrightness InitialBrightness() const noexcept override;

    void AddBlockModel(
        PartialSectionModelBuilderSet &modelBuilders,
        const Vec3i &blockPosition,
        const BlockNeighborhood blocks) const override;

    bool RayIntersect(
        const Vec3 &start, const Vec3 &dir, float minT, float maxT, Direction *pickedFace) const noexcept override;

    bool HasExtraData() const noexcept override;

    BlockExtraData CreateExtraData() const override;

    const LiquidDescription *GetLiquidDescription() const noexcept override;

private:

    std::string name_;
    LiquidDescription liquid_;
    BlockBrightness attenuation_;

    std::shared_ptr<const DefaultBlockEffect> effect_;
};

VRPG_GAME_END
