#include <VRPG/Game/Block/BasicDescription/DiffuseHollowBoxDescription.h>
#include <VRPG/Game/Utility/BoxModel.h>

VRPG_GAME_BEGIN

DiffuseHollowBoxDescription::DiffuseHollowBoxDescription(
    std::string name,
    std::shared_ptr<const DiffuseHollowBlockEffect> effect, int textureIndexInEffect[],
    BlockBrightness emission,
    BlockBrightness attenuation)
    : name_(std::move(name)), effect_(std::move(effect)),
      textureIndexInEffect_{
        textureIndexInEffect[0], textureIndexInEffect[1], textureIndexInEffect[2],
        textureIndexInEffect[3], textureIndexInEffect[4], textureIndexInEffect[5]
      },
      isLightSource_(emission != BLOCK_BRIGHTNESS_MIN), emission_(emission),
      attenuation_(attenuation)
{

}

const char *DiffuseHollowBoxDescription::GetName() const
{
    return name_.c_str();
}

FaceVisibilityProperty DiffuseHollowBoxDescription::GetFaceVisibilityProperty(Direction direction) const noexcept
{
    return FaceVisibilityProperty::Hollow;
}

bool DiffuseHollowBoxDescription::IsVisible() const noexcept
{
    return true;
}

bool DiffuseHollowBoxDescription::IsFullOpaque() const noexcept
{
    return false;
}

bool DiffuseHollowBoxDescription::IsLightSource() const noexcept
{
    return isLightSource_;
}

BlockBrightness DiffuseHollowBoxDescription::LightAttenuation() const noexcept
{
    return attenuation_;
}

BlockBrightness DiffuseHollowBoxDescription::InitialBrightness() const noexcept
{
    return emission_;
}

void DiffuseHollowBoxDescription::AddBlockModel(
    PartialSectionModelBuilderSet &modelBuilders,
    const Vec3i &blockPosition,
    const BlockNeighborhood blocks) const
{
    auto builder = modelBuilders.GetBuilderByEffect(effect_.get());
    Vec3 positionBase = blockPosition.map([](int i) { return float(i); });

    auto isFaceVisible = [&](int neiX, int neiY, int neiZ, Direction neiDir)
    {
        neiDir = blocks[neiX][neiY][neiZ].orientation.RotatedToOrigin(neiDir);
        FaceVisibilityProperty neiVis = blocks[neiX][neiY][neiZ].desc->GetFaceVisibilityProperty(neiDir);
        FaceVisibility visibility = TestFaceVisibility(FaceVisibilityProperty::Hollow, neiVis);
        return visibility == FaceVisibility::Yes || (visibility == FaceVisibility::Pos && !IsPositive(neiDir));
    };
    
    auto addFace = [&](
        const Vec3 &posA, const Vec3 &posB, const Vec3 &posC, const Vec3 &posD,
        const Vec4 &lhtA, const Vec4 &lhtB, const Vec4 &lhtC, const Vec4 &lhtD,
        uint32_t textureIndexInEffect)
    {
        Vec3 posE = 0.25f * (posA + posB + posC + posD);
        Vec4 lhtE = 0.25f * (lhtA + lhtB + lhtC + lhtD);

        VertexIndex vertexCount = VertexIndex(builder->GetVertexCount());

        Vec3 normal = cross(posB - posA, posC - posB).normalize();

        builder->AddVertex({ posA, BOX_FACE_TEXCOORD[0], normal, lhtA, textureIndexInEffect });
        builder->AddVertex({ posB, BOX_FACE_TEXCOORD[1], normal, lhtB, textureIndexInEffect });
        builder->AddVertex({ posC, BOX_FACE_TEXCOORD[2], normal, lhtC, textureIndexInEffect });
        builder->AddVertex({ posD, BOX_FACE_TEXCOORD[3], normal, lhtD, textureIndexInEffect });
        builder->AddVertex({ posE, Vec2(0.5f, 0.5f), normal, lhtE, textureIndexInEffect });

        builder->AddIndexedTriangle(vertexCount + 0, vertexCount + 1, vertexCount + 4);
        builder->AddIndexedTriangle(vertexCount + 1, vertexCount + 2, vertexCount + 4);
        builder->AddIndexedTriangle(vertexCount + 2, vertexCount + 3, vertexCount + 4);
        builder->AddIndexedTriangle(vertexCount + 3, vertexCount + 0, vertexCount + 4);
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
