#include <VRPG/Game/Block/BasicDescription/TransparentBoxDescription.h>
#include <VRPG/Game/Utility/BoxModel.h>

VRPG_GAME_BEGIN

TransparentBoxDescription::TransparentBoxDescription(
    std::string name,
    std::shared_ptr<TransparentBlockEffect> effect, int textureIndexInEffect[],
    BlockBrightness attenuation)
    : name_(std::move(name)), effect_(std::move(effect)),
      textureIndexInEffect_{
          textureIndexInEffect[0], textureIndexInEffect[1], textureIndexInEffect[2],
          textureIndexInEffect[3], textureIndexInEffect[4], textureIndexInEffect[5]
      },
      attenuation_(attenuation)
{

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
    const BlockNeighborhood blocks) const
{
    auto builder = modelBuilders.GetBuilderByEffect(effect_.get());
    Vec3 positionBase = blockPosition.map([](int i) { return float(i); });

    auto isFaceVisible = [&](int neiX, int neiY, int neiZ, Direction neiDir)
    {
        auto neiDesc = blocks[neiX][neiY][neiZ].desc;
        neiDir = blocks[neiX][neiY][neiZ].orientation.RotatedToOrigin(neiDir);
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

        builder->AddVertex({ posA, BOX_FACE_TEXCOORD[0], textureIndexInEffect, lhtA });
        builder->AddVertex({ posB, BOX_FACE_TEXCOORD[1], textureIndexInEffect, lhtB });
        builder->AddVertex({ posC, BOX_FACE_TEXCOORD[2], textureIndexInEffect, lhtC });
        builder->AddVertex({ posD, BOX_FACE_TEXCOORD[3], textureIndexInEffect, lhtD });
        builder->AddVertex({ posE, Vec2(0.5f, 0.5f), textureIndexInEffect, lhtE });

        builder->AddIndexedTriangle(vertexCount + 0, vertexCount + 1, vertexCount + 4);
        builder->AddIndexedTriangle(vertexCount + 1, vertexCount + 2, vertexCount + 4);
        builder->AddIndexedTriangle(vertexCount + 2, vertexCount + 3, vertexCount + 4);
        builder->AddIndexedTriangle(vertexCount + 3, vertexCount + 0, vertexCount + 4);

        builder->AddFaceIndexRange(posE, startIndex);
    };

    BlockOrientation orientation = blocks[1][1][1].orientation;

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

        Vec4 light0 = BoxVertexBrightness(blocks, rotDir, position[0]);
        Vec4 light1 = BoxVertexBrightness(blocks, rotDir, position[1]);
        Vec4 light2 = BoxVertexBrightness(blocks, rotDir, position[2]);
        Vec4 light3 = BoxVertexBrightness(blocks, rotDir, position[3]);
        
        addFace(positionBase + position[0],
                positionBase + position[1],
                positionBase + position[2],
                positionBase + position[3],
                light0, light1, light2, light3, textureIndexInEffect_[int(normalDirection)]);
    };

    generateFace(PositiveX);
    generateFace(NegativeX);
    generateFace(PositiveY);
    generateFace(NegativeY);
    generateFace(PositiveZ);
    generateFace(NegativeZ);
}

VRPG_GAME_END
