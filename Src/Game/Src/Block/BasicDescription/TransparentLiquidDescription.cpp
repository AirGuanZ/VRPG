#include <VRPG/Game/Block/BasicDescription/TransparentLiquidDescription.h>
#include "VRPG/Game/Utility/BoxModel.h"

VRPG_GAME_BEGIN

TransparentLiquidDescription::TransparentLiquidDescription(
    std::string name, LiquidDescription liquid, BlockBrightness attenuation)
    : name_(std::move(name)), liquid_(liquid), attenuation_(attenuation)
{
    effect_ = std::dynamic_pointer_cast<const DefaultBlockEffect>(
        BlockEffectManager::GetInstance().GetSharedBlockEffectByName("default"));
}

const char *TransparentLiquidDescription::GetName() const
{
    return name_.c_str();
}

FaceVisibilityProperty TransparentLiquidDescription::GetFaceVisibilityProperty(Direction direction) const noexcept
{
    return FaceVisibilityProperty::Transparent;
}

bool TransparentLiquidDescription::IsVisible() const noexcept
{
    return true;
}

bool TransparentLiquidDescription::IsReplacable() const noexcept
{
    return true;
}

bool TransparentLiquidDescription::IsFullOpaque() const noexcept
{
    return false;
}

bool TransparentLiquidDescription::IsLightSource() const noexcept
{
    return false;
}

BlockBrightness TransparentLiquidDescription::LightAttenuation() const noexcept
{
    return attenuation_;
}

BlockBrightness TransparentLiquidDescription::InitialBrightness() const noexcept
{
    return BLOCK_BRIGHTNESS_MIN;
}

void TransparentLiquidDescription::AddBlockModel(
    PartialSectionModelBuilderSet &modelBuilders,
    const Vec3i &blockPosition,
    const BlockNeighborhood blocks) const
{
    auto builder = modelBuilders.GetBuilderByEffect(effect_.get());
    Vec3 positionBase = blockPosition.map([](int i) { return float(i); });

    auto isFaceVisible = [&](Direction dirToNei)
    {
        auto [x, y, z] = DirectionToVectori(dirToNei) + Vec3i(1);
        auto neiDesc = blocks[x][y][z].desc;
        Direction neiDir = blocks[x][y][z].orientation.RotatedToOrigin(-dirToNei);
        FaceVisibilityProperty neiVis = neiDesc->GetFaceVisibilityProperty(neiDir);
        FaceVisibility vis = TestFaceVisibility(FaceVisibilityProperty::Transparent, neiVis);
        return vis == FaceVisibility::Yes || (vis == FaceVisibility::Diff && this != neiDesc);
    };

    auto addFace = [&](
        const Vec3 &posA, const Vec3 &posB, const Vec3 &posC, const Vec3 &posD,
        const Vec4 &lhtA, const Vec4 &lhtB, const Vec4 &lhtC, const Vec4 &lhtD)
    {
        Vec3 posE = 0.25f * (posA + posB + posC + posD);
        Vec4 lhtE = 0.25f * (lhtA + lhtB + lhtC + lhtD);

        VertexIndex vertexCount = VertexIndex(builder->GetVertexCount());

        builder->AddVertex({ posA, lhtA });
        builder->AddVertex({ posB, lhtB });
        builder->AddVertex({ posC, lhtC });
        builder->AddVertex({ posD, lhtD });
        builder->AddVertex({ posE, lhtE });

        builder->AddIndexedTriangle(vertexCount + 0, vertexCount + 1, vertexCount + 4);
        builder->AddIndexedTriangle(vertexCount + 1, vertexCount + 2, vertexCount + 4);
        builder->AddIndexedTriangle(vertexCount + 2, vertexCount + 3, vertexCount + 4);
        builder->AddIndexedTriangle(vertexCount + 3, vertexCount + 0, vertexCount + 4);
    };

    BlockOrientation orientation = blocks[1][1][1].orientation;

    float vertexHeight = 1;
    auto liquid = blocks[1][1][1].desc->GetLiquidDescription();
    if(liquid->IsSource(*blocks[1][1][1].extraData))
    {
        if(blocks[1][2][1].desc != blocks[1][1][1].desc)
            vertexHeight = liquid->TopSourceHeight();
    }
    else
        vertexHeight = liquid->LevelToVertexHeight(ExtraDataToLiquidLevel(*blocks[1][1][1].extraData));

    auto generateFace = [&](Direction normalDirection)
    {
        Direction rotDir = orientation.OriginToRotated(normalDirection);
        if(!isFaceVisible(rotDir))
            return;

        Vec3 position[4];
        GenerateBoxFaceDynamic(normalDirection, position);
        position[0] = RotateLocalPosition(orientation, position[0]);
        position[1] = RotateLocalPosition(orientation, position[1]);
        position[2] = RotateLocalPosition(orientation, position[2]);
        position[3] = RotateLocalPosition(orientation, position[3]);

        position[0].y *= vertexHeight;
        position[1].y *= vertexHeight;
        position[2].y *= vertexHeight;
        position[3].y *= vertexHeight;

        Vec4 light0 = BoxVertexBrightness(blocks, rotDir, position[0]);
        Vec4 light1 = BoxVertexBrightness(blocks, rotDir, position[1]);
        Vec4 light2 = BoxVertexBrightness(blocks, rotDir, position[2]);
        Vec4 light3 = BoxVertexBrightness(blocks, rotDir, position[3]);

        addFace(positionBase + position[0],
                positionBase + position[1],
                positionBase + position[2],
                positionBase + position[3],
                light0, light1, light2, light3);
    };

    generateFace(PositiveX);
    generateFace(NegativeX);
    generateFace(PositiveY);
    generateFace(NegativeY);
    generateFace(PositiveZ);
    generateFace(NegativeZ);
}

bool TransparentLiquidDescription::RayIntersect(
    const Vec3 &start, const Vec3 &dir, float minT, float maxT, Direction *pickedFace) const noexcept
{
    return false;
}

bool TransparentLiquidDescription::HasExtraData() const noexcept
{
    return true;
}

BlockExtraData TransparentLiquidDescription::CreateExtraData() const
{
    return MakeLiquidExtraData(0);
}

const LiquidDescription *TransparentLiquidDescription::GetLiquidDescription() const noexcept
{
    return &liquid_;
}

VRPG_GAME_END
