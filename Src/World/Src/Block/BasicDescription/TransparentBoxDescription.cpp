#include <VRPG/World/Block/BasicDescription/TransparentBoxDescription.h>
#include <VRPG/World/Utility/BoxModel.h>

VRPG_WORLD_BEGIN

TransparentBoxDescription::TransparentBoxDescription(
    std::string name,
    std::shared_ptr<TransparentBlockEffect> effect, int textureIndexInEffect[],
    BlockBrightness attenuation)
    : textureIndexInEffect_{}
{
    name_ = std::move(name);
    effect_ = std::move(effect);
    for(int i = 0; i < 6; ++i)
        textureIndexInEffect_[i] = textureIndexInEffect[i];
    attenuation_ = attenuation;
}

const char *TransparentBoxDescription::GetName() const
{
    return name_.c_str();
}

FaceVisibilityProperty TransparentBoxDescription::GetFaceVisibilityProperty(Direction direction) const noexcept
{
    return FaceVisibilityProperty::Transparent;
}

bool TransparentBoxDescription::IsVisible() const noexcept
{
    return true;
}

bool TransparentBoxDescription::IsFullOpaque() const noexcept
{
    return false;
}

bool TransparentBoxDescription::IsLightSource() const noexcept
{
    return false;
}

BlockBrightness TransparentBoxDescription::LightAttenuation() const noexcept
{
    return attenuation_;
}

BlockBrightness TransparentBoxDescription::InitialBrightness() const noexcept
{
    return BLOCK_BRIGHTNESS_MIN;
}

void TransparentBoxDescription::AddBlockModel(
    PartialSectionModelBuilderSet &modelBuilders,
    const Vec3i &blockPosition,
    const BlockDescription *neighborBlocks[3][3][3],
    const BlockBrightness neighborBrightness[3][3][3],
    const BlockOrientation neighborOrientations[3][3][3]) const
{
    auto builder = modelBuilders.GetBuilderByEffect(effect_.get());
    Vec3 positionBase = blockPosition.map([](int i) { return float(i); });

    auto isFaceVisible = [&](int neiX, int neiY, int neiZ, Direction neiDir)
    {
        auto neiDesc = neighborBlocks[neiX][neiY][neiZ];
        neiDir = neighborOrientations[neiX][neiY][neiZ].OriginToRotated(neiDir);
        FaceVisibilityProperty neiVis = neiDesc->GetFaceVisibilityProperty(neiDir);
        FaceVisibility vis = IsFaceVisible(FaceVisibilityProperty::Transparent, neiVis);
        return vis == FaceVisibility::Yes || (vis == FaceVisibility::Diff && neiDesc != this);
    };
    
    auto addFace = [&](
        const Vec3 &posA, const Vec3 &posB, const Vec3 &posC, const Vec3 &posD,
        const Vec4 &lhtA, const Vec4 &lhtB, const Vec4 &lhtC, const Vec4 &lhtD,
        uint32_t textureIndexInEffect)
    {
        Vec3 posE = 0.25f * (posA + posB + posC + posD);
        Vec4 lhtE = 0.25f * (lhtA + lhtB + lhtC + lhtD);

        VertexIndex vertexCount = VertexIndex(builder->GetVertexCount());
        VertexIndex startIndex = VertexIndex(builder->GetIndexCount());

        builder->AddVertex({ posA, Vec2(0, 1),       textureIndexInEffect, lhtA });
        builder->AddVertex({ posB, Vec2(0, 0),       textureIndexInEffect, lhtB });
        builder->AddVertex({ posC, Vec2(1, 0),       textureIndexInEffect, lhtC });
        builder->AddVertex({ posD, Vec2(1, 1),       textureIndexInEffect, lhtD });
        builder->AddVertex({ posE, Vec2(0.5f, 0.5f), textureIndexInEffect, lhtE });

        builder->AddIndexedTriangle(vertexCount + 0, vertexCount + 1, vertexCount + 4);
        builder->AddIndexedTriangle(vertexCount + 1, vertexCount + 2, vertexCount + 4);
        builder->AddIndexedTriangle(vertexCount + 2, vertexCount + 3, vertexCount + 4);
        builder->AddIndexedTriangle(vertexCount + 3, vertexCount + 0, vertexCount + 4);

        builder->AddFaceIndexRange(posE, startIndex);
    };

    // +x
    if(isFaceVisible(2, 1, 1, NegativeX))
    {
        Vec3 posA = positionBase + Vec3(1, 0, 0);
        Vec3 posB = positionBase + Vec3(1, 1, 0);
        Vec3 posC = positionBase + Vec3(1, 1, 1);
        Vec3 posD = positionBase + Vec3(1, 0, 1);

        Vec4 lightA = BoxVertexBrightness_X(neighborBlocks, neighborBrightness, 2, -1, -1);
        Vec4 lightB = BoxVertexBrightness_X(neighborBlocks, neighborBrightness, 2, 1, -1);
        Vec4 lightC = BoxVertexBrightness_X(neighborBlocks, neighborBrightness, 2, 1, 1);
        Vec4 lightD = BoxVertexBrightness_X(neighborBlocks, neighborBrightness, 2, -1, 1);

        addFace(posA, posB, posC, posD, lightA, lightB, lightC, lightD, textureIndexInEffect_[PositiveX]);
    }

    // -x
    if(isFaceVisible(0, 1, 1, PositiveX))
    {
        Vec3 posA = positionBase + Vec3(0, 0, 1);
        Vec3 posB = positionBase + Vec3(0, 1, 1);
        Vec3 posC = positionBase + Vec3(0, 1, 0);
        Vec3 posD = positionBase + Vec3(0, 0, 0);

        Vec4 lightA = BoxVertexBrightness_X(neighborBlocks, neighborBrightness, 0, -1, 1);
        Vec4 lightB = BoxVertexBrightness_X(neighborBlocks, neighborBrightness, 0, 1, 1);
        Vec4 lightC = BoxVertexBrightness_X(neighborBlocks, neighborBrightness, 0, 1, -1);
        Vec4 lightD = BoxVertexBrightness_X(neighborBlocks, neighborBrightness, 0, -1, -1);

        addFace(posA, posB, posC, posD, lightA, lightB, lightC, lightD, textureIndexInEffect_[NegativeX]);
    }

    // +y
    if(isFaceVisible(1, 2, 1, NegativeY))
    {
        Vec3 posA = positionBase + Vec3(0, 1, 0);
        Vec3 posB = positionBase + Vec3(0, 1, 1);
        Vec3 posC = positionBase + Vec3(1, 1, 1);
        Vec3 posD = positionBase + Vec3(1, 1, 0);

        Vec4 lightA = BoxVertexBrightness_Y(neighborBlocks, neighborBrightness, -1, 2, -1);
        Vec4 lightB = BoxVertexBrightness_Y(neighborBlocks, neighborBrightness, -1, 2, 1);
        Vec4 lightC = BoxVertexBrightness_Y(neighborBlocks, neighborBrightness, 1, 2, 1);
        Vec4 lightD = BoxVertexBrightness_Y(neighborBlocks, neighborBrightness, 1, 2, -1);

        addFace(posA, posB, posC, posD, lightA, lightB, lightC, lightD, textureIndexInEffect_[PositiveY]);
    }

    // -y
    if(isFaceVisible(1, 0, 1, PositiveY))
    {
        Vec3 posA = positionBase + Vec3(1, 0, 0);
        Vec3 posB = positionBase + Vec3(1, 0, 1);
        Vec3 posC = positionBase + Vec3(0, 0, 1);
        Vec3 posD = positionBase + Vec3(0, 0, 0);

        Vec4 lightA = BoxVertexBrightness_Y(neighborBlocks, neighborBrightness, 1, 0, -1);
        Vec4 lightB = BoxVertexBrightness_Y(neighborBlocks, neighborBrightness, 1, 0, 1);
        Vec4 lightC = BoxVertexBrightness_Y(neighborBlocks, neighborBrightness, -1, 0, 1);
        Vec4 lightD = BoxVertexBrightness_Y(neighborBlocks, neighborBrightness, -1, 0, -1);

        addFace(posA, posB, posC, posD, lightA, lightB, lightC, lightD, textureIndexInEffect_[NegativeY]);
    }

    // +z
    if(isFaceVisible(1, 1, 2, NegativeZ))
    {
        Vec3 posA = positionBase + Vec3(1, 0, 1);
        Vec3 posB = positionBase + Vec3(1, 1, 1);
        Vec3 posC = positionBase + Vec3(0, 1, 1);
        Vec3 posD = positionBase + Vec3(0, 0, 1);

        Vec4 lightA = BoxVertexBrightness_Z(neighborBlocks, neighborBrightness, 1, -1, 2);
        Vec4 lightB = BoxVertexBrightness_Z(neighborBlocks, neighborBrightness, 1, 1, 2);
        Vec4 lightC = BoxVertexBrightness_Z(neighborBlocks, neighborBrightness, -1, 1, 2);
        Vec4 lightD = BoxVertexBrightness_Z(neighborBlocks, neighborBrightness, -1, -1, 2);

        addFace(posA, posB, posC, posD, lightA, lightB, lightC, lightD, textureIndexInEffect_[PositiveZ]);
    }

    // -z
    if(isFaceVisible(1, 1, 0, PositiveZ))
    {
        Vec3 posA = positionBase + Vec3(0, 0, 0);
        Vec3 posB = positionBase + Vec3(0, 1, 0);
        Vec3 posC = positionBase + Vec3(1, 1, 0);
        Vec3 posD = positionBase + Vec3(1, 0, 0);

        Vec4 lightA = BoxVertexBrightness_Z(neighborBlocks, neighborBrightness, -1, -1, 0);
        Vec4 lightB = BoxVertexBrightness_Z(neighborBlocks, neighborBrightness, -1, 1, 0);
        Vec4 lightC = BoxVertexBrightness_Z(neighborBlocks, neighborBrightness, 1, 1, 0);
        Vec4 lightD = BoxVertexBrightness_Z(neighborBlocks, neighborBrightness, 1, -1, 0);

        addFace(posA, posB, posC, posD, lightA, lightB, lightC, lightD, textureIndexInEffect_[NegativeZ]);
    }
}

VRPG_WORLD_END
