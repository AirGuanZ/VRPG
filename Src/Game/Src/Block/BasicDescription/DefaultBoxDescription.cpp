#include <VRPG/Game/Block/BasicDescription/DefaultBoxDescription.h>
#include <VRPG/Game/Block/BasicEffect/DefaultBlockEffect.h>
#include <VRPG/Game/Utility/BoxModel.h>

VRPG_GAME_BEGIN

DefaultBlockDescription::DefaultBlockDescription(std::shared_ptr<const DefaultBlockEffect> effect) noexcept
    : effect_(std::move(effect))
{
    
}

FaceVisibilityProperty DefaultBlockDescription::GetFaceVisibilityProperty(Direction direction) const noexcept
{
    return FaceVisibilityProperty::Solid;
}

bool DefaultBlockDescription::IsVisible() const noexcept
{
    return true;
}

bool DefaultBlockDescription::IsFullOpaque() const noexcept
{
    return true;
}

bool DefaultBlockDescription::IsLightSource() const noexcept
{
    return false;
}

BlockBrightness DefaultBlockDescription::LightAttenuation() const noexcept
{
    return BLOCK_BRIGHTNESS_MAX;
}

BlockBrightness DefaultBlockDescription::InitialBrightness() const noexcept
{
    return BLOCK_BRIGHTNESS_MIN;
}

void DefaultBlockDescription::AddBlockModel(
    PartialSectionModelBuilderSet &modelBuilders,
    const Vec3i &blockPosition,
    const BlockNeighborhood neighborhood) const
{
    auto builder = modelBuilders.GetBuilderByEffect(effect_.get());
    Vec3 positionBase = blockPosition.map([](int i) { return float(i); });

    auto addFace = [&](
        const Vec3 &posA, const Vec3 &posB, const Vec3 &posC, const Vec3 &posD,
        const Vec4 &lhtA, const Vec4 &lhtB, const Vec4 &lhtC, const Vec4 &lhtD)
    {
        Vec3 posE = 0.25f * (posA + posB + posC + posD);
        Vec4 lhtE = 0.25f * (lhtA + lhtB+  lhtC + lhtD);

        VertexIndex vertexCount = VertexIndex(builder->GetVertexCount());

        Vec3 normal = cross(posB - posA, posC - posB).normalize();

        builder->AddVertex({ posA, lhtA, normal });
        builder->AddVertex({ posB, lhtB, normal });
        builder->AddVertex({ posC, lhtC, normal });
        builder->AddVertex({ posD, lhtD, normal });
        builder->AddVertex({ posE, lhtE, normal });

        builder->AddIndexedTriangle(vertexCount + 0, vertexCount + 1, vertexCount + 4);
        builder->AddIndexedTriangle(vertexCount + 1, vertexCount + 2, vertexCount + 4);
        builder->AddIndexedTriangle(vertexCount + 2, vertexCount + 3, vertexCount + 4);
        builder->AddIndexedTriangle(vertexCount + 3, vertexCount + 0, vertexCount + 4);
    };

    auto isFaceVisible = [&](int neiX, int neiY, int neiZ, Direction neiDir)
    {
        neiDir = neighborhood[neiX][neiY][neiZ].orientation.RotatedToOrigin(neiDir);
        FaceVisibilityProperty neiVis = neighborhood[neiX][neiY][neiZ].desc->GetFaceVisibilityProperty(neiDir);
        FaceVisibility visibility = TestFaceVisibility(FaceVisibilityProperty::Solid, neiVis);
        return visibility == FaceVisibility::Yes;
    };

    BlockOrientation orientation = neighborhood[1][1][1].orientation;

    auto generateFace = [&](Direction normalDirection)
    {
        Direction rotDir = orientation.OriginToRotated(normalDirection);;
        static const Vec3i ROT_DIR_TO_NEI_INDEX[6] =
        {
            { 2, 1, 1 }, { 0, 1, 1 },
            { 1, 2, 1 }, { 1, 0, 1 },
            { 1, 1, 2 }, { 1, 1, 0 }
        };
        Vec3i neiIndex = ROT_DIR_TO_NEI_INDEX[int(rotDir)];
        if(!isFaceVisible(neiIndex.x, neiIndex.y, neiIndex.z, -rotDir))
            return;

        Vec3 position[4];
        GenerateBoxFaceDynamic(normalDirection, position);
        position[0] = RotateLocalPosition(orientation, position[0]);
        position[1] = RotateLocalPosition(orientation, position[1]);
        position[2] = RotateLocalPosition(orientation, position[2]);
        position[3] = RotateLocalPosition(orientation, position[3]);

        Vec4 light0 = BoxVertexBrightness(neighborhood, rotDir, position[0]);
        Vec4 light1 = BoxVertexBrightness(neighborhood, rotDir, position[1]);
        Vec4 light2 = BoxVertexBrightness(neighborhood, rotDir, position[2]);
        Vec4 light3 = BoxVertexBrightness(neighborhood, rotDir, position[3]);

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

VRPG_GAME_END
