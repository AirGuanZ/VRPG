#include <VRPG/World/Block/BasicDescription/DefaultBoxDescription.h>
#include <VRPG/World/Block/BasicEffect/DefaultBlockEffect.h>
#include <VRPG/World/Utility/BoxVertexBrightness.h>

VRPG_WORLD_BEGIN

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
    const BlockDescription *neighborBlocks[3][3][3],
    const BlockBrightness neighborBrightness[3][3][3],
    const BlockOrientation neighborOrientations[3][3][3]) const
{
    auto builder = modelBuilders.GetBuilderByEffect(effect_.get());
    Vec3 blockPositionf = blockPosition.map([](int i) { return float(i); });

    auto addFace = [&](
        const Vec3 &posA, const Vec3 &posB, const Vec3 &posC, const Vec3 &posD,
        const Vec4 &lhtA, const Vec4 &lhtB, const Vec4 &lhtC, const Vec4 &lhtD)
    {
        Vec3 posE = 0.25f * (posA + posB + posC + posD);
        Vec4 lhtE = 0.25f * (lhtA + lhtB+  lhtC + lhtD);

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

    auto isFaceVisible = [&](int neiX, int neiY, int neiZ, Direction neiDir)
    {
        neiDir = neighborOrientations[neiX][neiY][neiZ].OriginToRotated(neiDir);
        FaceVisibilityProperty neiVis = neighborBlocks[neiX][neiY][neiZ]->GetFaceVisibilityProperty(neiDir);
        FaceVisibility visibility = IsFaceVisible(FaceVisibilityProperty::Solid, neiVis);
        return visibility == FaceVisibility::Yes;
    };

    // +x
    if(isFaceVisible(2, 1, 1, NegativeX))
    {
        Vec3 posA = blockPositionf + Vec3(1, 0, 0);
        Vec3 posB = blockPositionf + Vec3(1, 1, 0);
        Vec3 posC = blockPositionf + Vec3(1, 1, 1);
        Vec3 posD = blockPositionf + Vec3(1, 0, 1);

        Vec4 lightA = BoxVertexBrightness_X(neighborBlocks, neighborBrightness, 2, -1, -1);
        Vec4 lightB = BoxVertexBrightness_X(neighborBlocks, neighborBrightness, 2, 1, -1);
        Vec4 lightC = BoxVertexBrightness_X(neighborBlocks, neighborBrightness, 2, 1, 1);
        Vec4 lightD = BoxVertexBrightness_X(neighborBlocks, neighborBrightness, 2, -1, 1);

        addFace(posA, posB, posC, posD, lightA, lightB, lightC, lightD);
    }

    // -x
    if(isFaceVisible(0, 1, 1, PositiveX))
    {
        Vec3 posA = blockPositionf + Vec3(0, 0, 1);
        Vec3 posB = blockPositionf + Vec3(0, 1, 1);
        Vec3 posC = blockPositionf + Vec3(0, 1, 0);
        Vec3 posD = blockPositionf + Vec3(0, 0, 0);

        Vec4 lightA = BoxVertexBrightness_X(neighborBlocks, neighborBrightness, 0, -1, 1);
        Vec4 lightB = BoxVertexBrightness_X(neighborBlocks, neighborBrightness, 0, 1, 1);
        Vec4 lightC = BoxVertexBrightness_X(neighborBlocks, neighborBrightness, 0, 1, -1);
        Vec4 lightD = BoxVertexBrightness_X(neighborBlocks, neighborBrightness, 0, -1, -1);

        addFace(posA, posB, posC, posD, lightA, lightB, lightC, lightD);
    }

    // +y
    if(isFaceVisible(1, 2, 1, NegativeY))
    {
        Vec3 posA = blockPositionf + Vec3(0, 1, 0);
        Vec3 posB = blockPositionf + Vec3(0, 1, 1);
        Vec3 posC = blockPositionf + Vec3(1, 1, 1);
        Vec3 posD = blockPositionf + Vec3(1, 1, 0);

        Vec4 lightA = BoxVertexBrightness_Y(neighborBlocks, neighborBrightness, -1, 2, -1);
        Vec4 lightB = BoxVertexBrightness_Y(neighborBlocks, neighborBrightness, -1, 2, 1);
        Vec4 lightC = BoxVertexBrightness_Y(neighborBlocks, neighborBrightness, 1, 2, 1);
        Vec4 lightD = BoxVertexBrightness_Y(neighborBlocks, neighborBrightness, 1, 2, -1);

        addFace(posA, posB, posC, posD, lightA, lightB, lightC, lightD);
    }

    // -y
    if(isFaceVisible(1, 0, 1, PositiveY))
    {
        Vec3 posA = blockPositionf + Vec3(1, 0, 0);
        Vec3 posB = blockPositionf + Vec3(1, 0, 1);
        Vec3 posC = blockPositionf + Vec3(0, 0, 1);
        Vec3 posD = blockPositionf + Vec3(0, 0, 0);

        Vec4 lightA = BoxVertexBrightness_Y(neighborBlocks, neighborBrightness, 1, 0, -1);
        Vec4 lightB = BoxVertexBrightness_Y(neighborBlocks, neighborBrightness, 1, 0, 1);
        Vec4 lightC = BoxVertexBrightness_Y(neighborBlocks, neighborBrightness, -1, 0, 1);
        Vec4 lightD = BoxVertexBrightness_Y(neighborBlocks, neighborBrightness, -1, 0, -1);

        addFace(posA, posB, posC, posD, lightA, lightB, lightC, lightD);
    }

    // +z
    if(isFaceVisible(1, 1, 2, NegativeZ))
    {
        Vec3 posA = blockPositionf + Vec3(1, 0, 1);
        Vec3 posB = blockPositionf + Vec3(1, 1, 1);
        Vec3 posC = blockPositionf + Vec3(0, 1, 1);
        Vec3 posD = blockPositionf + Vec3(0, 0, 1);

        Vec4 lightA = BoxVertexBrightness_Z(neighborBlocks, neighborBrightness, 1, -1, 2);
        Vec4 lightB = BoxVertexBrightness_Z(neighborBlocks, neighborBrightness, 1, 1, 2);
        Vec4 lightC = BoxVertexBrightness_Z(neighborBlocks, neighborBrightness, -1, 1, 2);
        Vec4 lightD = BoxVertexBrightness_Z(neighborBlocks, neighborBrightness, -1, -1, 2);

        addFace(posA, posB, posC, posD, lightA, lightB, lightC, lightD);
    }

    // -z
    if(isFaceVisible(1, 1, 0, PositiveZ))
    {
        Vec3 posA = blockPositionf + Vec3(0, 0, 0);
        Vec3 posB = blockPositionf + Vec3(0, 1, 0);
        Vec3 posC = blockPositionf + Vec3(1, 1, 0);
        Vec3 posD = blockPositionf + Vec3(1, 0, 0);

        Vec4 lightA = BoxVertexBrightness_Z(neighborBlocks, neighborBrightness, -1, -1, 0);
        Vec4 lightB = BoxVertexBrightness_Z(neighborBlocks, neighborBrightness, -1, 1, 0);
        Vec4 lightC = BoxVertexBrightness_Z(neighborBlocks, neighborBrightness, 1, 1, 0);
        Vec4 lightD = BoxVertexBrightness_Z(neighborBlocks, neighborBrightness, 1, -1, 0);

        addFace(posA, posB, posC, posD, lightA, lightB, lightC, lightD);
    }
}

VRPG_WORLD_END
