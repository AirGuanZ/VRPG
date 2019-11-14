#include <VRPG/Game/Block/BasicDescription/TransparentLiquidDescription.h>
#include "VRPG/Game/Utility/BoxModel.h"

VRPG_GAME_BEGIN

TransparentLiquidDescription::TransparentLiquidDescription(
    std::string name, LiquidDescription liquid,
    std::shared_ptr<const TransparentBlockEffect> effect, int textureIndexInEffect,
    BlockBrightness attenuation)
    : name_(std::move(name)), liquid_(liquid),
      effect_(std::move(effect)), textureIndexInEffect_(textureIndexInEffect),
      attenuation_(attenuation)
{
    
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
        const Vec4 &lhtA, const Vec4 &lhtB, const Vec4 &lhtC, const Vec4 &lhtD,
        const Vec3 &sortCentre)
    {
        Vec3 posE = 0.25f * (posA + posB + posC + posD);
        Vec4 lhtE = 0.25f * (lhtA + lhtB + lhtC + lhtD);

        VertexIndex vertexCount = VertexIndex(builder->GetVertexCount());
        VertexIndex startIndex = VertexIndex(builder->GetIndexCount());

        builder->AddVertex({ positionBase + posA, Vec2(0.5f), uint32_t(textureIndexInEffect_), lhtA });
        builder->AddVertex({ positionBase + posB, Vec2(0.5f), uint32_t(textureIndexInEffect_), lhtB });
        builder->AddVertex({ positionBase + posC, Vec2(0.5f), uint32_t(textureIndexInEffect_), lhtC });
        builder->AddVertex({ positionBase + posD, Vec2(0.5f), uint32_t(textureIndexInEffect_), lhtD });
        builder->AddVertex({ positionBase + posE, Vec2(0.5f), uint32_t(textureIndexInEffect_), lhtE });

        builder->AddIndexedTriangle(vertexCount + 0, vertexCount + 1, vertexCount + 4);
        builder->AddIndexedTriangle(vertexCount + 1, vertexCount + 2, vertexCount + 4);
        builder->AddIndexedTriangle(vertexCount + 2, vertexCount + 3, vertexCount + 4);
        builder->AddIndexedTriangle(vertexCount + 3, vertexCount + 0, vertexCount + 4);

        builder->AddFaceIndexRange(positionBase + sortCentre, startIndex);
    };

    bool isThisSource = blocks[1][1][1].desc->GetLiquidDescription()->IsSource(*blocks[1][1][1].extraData);
    bool isUpSame = blocks[1][2][1].desc == this;

    // 计算四个xz角点处的液面高度
    float vertexHeights[2][2];
    {
        auto vertexHeight = [&](int x, int z) -> float
        {
            auto &block = blocks[x][1][z];
            if(!block.desc->IsLiquid())
                return 0;

            auto liquid = block.desc->GetLiquidDescription();
            if(liquid->IsSource(*block.extraData))
            {
                if(blocks[x][2][z].desc != block.desc)
                    return liquid->TopSourceHeight();
                return 1;
            }
            
            return liquid->LevelToVertexHeight(ExtraDataToLiquidLevel(*block.extraData));
        };

        float blockHeights[3][3];
        for(int x = 0; x < 3; ++x)
        {
            for(int z = 0; z < 3; ++z)
            {
                blockHeights[x][z] = vertexHeight(x, z);
            }
        }

        auto synthesisVertexHeight = [&](int dx, int dz) -> float
        {
            if(!isThisSource && !isUpSame && blocks[1 + dx][1][1].desc->IsVoid() && blocks[1][1][1 + dz].desc->IsVoid())
                return 0;
            return (std::max)((std::max)(blockHeights[1][1], blockHeights[1 + dx][1 + dz]),
                              (std::max)(blockHeights[1 + dx][1], blockHeights[1][1 + dz]));
        };

        vertexHeights[0][0] = synthesisVertexHeight(-1, -1);
        vertexHeights[0][1] = synthesisVertexHeight(-1, +1);
        vertexHeights[1][0] = synthesisVertexHeight(+1, -1);
        vertexHeights[1][1] = synthesisVertexHeight(+1, +1);
    }

    auto vertexBrightness = [&](Direction normalDirection, const Vec3 &pos)
    {
        if(pos.y < 0.001f || pos.y > 0.999f)
            return BoxVertexBrightness(blocks, normalDirection, pos);

        assert(normalDirection != NegativeY);
        if(normalDirection == PositiveY)
        {
            int dx = pos.x > 0.5f ? 1 : -1;
            int dz = pos.z > 0.5f ? 1 : -1;

            int count = 1;
            Vec4 sum = BlockBrightnessToFloat(blocks[1][1][1].brightness);
            if(blocks[1 + dx][1][1].desc == this)
            {
                ++count;
                sum += BlockBrightnessToFloat(blocks[1 + dx][1][1].brightness);
            }
            if(blocks[1][1][1 + dz].desc == this)
            {
                ++count;
                sum += BlockBrightnessToFloat(blocks[1][1][1 + dz].brightness);
            }
            if(count > 1 && blocks[1 + dx][1][1 + dz].desc == this)
            {
                ++count;
                sum += BlockBrightnessToFloat(blocks[1 + dx][1][1 + dz].brightness);
            }

            return ComputeVertexBrightness(sum) / float(count);
        }

        int dx = (normalDirection == PositiveX || normalDirection == NegativeX) ? 0 : (pos.x > 0.5f ? 1 : -1);
        int dz = (normalDirection == PositiveZ || normalDirection == NegativeZ) ? 0 : (pos.z > 0.5f ? 1 : -1);

        Vec3i b = Vec3i(1) + DirectionToVectori(normalDirection);
        return (0.5f + 0.5f * SIDE_VERTEX_BRIGHTNESS_RATIO) * ComputeVertexBrightness(
            blocks[b.x     ][1][b.z     ].brightness,
            blocks[b.x + dx][1][b.z + dz].brightness);
    };

    auto generateFace = [&](Direction normalDirection)
    {
        if(!isFaceVisible(normalDirection))
            return;

        Vec3i pos[4]; Vec3 posf[4];
        GenerateBoxFaceiDynamic(normalDirection, pos);

        Vec3 posSum;
        for(int i = 0; i < 4; ++i)
        {
            int xi = pos[i].x, zi = pos[i].z;
            posf[i] = { float(xi), pos[i].y * vertexHeights[xi][zi], float(zi) };
            posSum += pos[i].map([](int i) { return float(i); });
        }

        Vec4 lhtA = vertexBrightness(normalDirection, posf[0]);
        Vec4 lhtB = vertexBrightness(normalDirection, posf[1]);
        Vec4 lhtC = vertexBrightness(normalDirection, posf[2]);
        Vec4 lhtD = vertexBrightness(normalDirection, posf[3]);

        addFace(posf[0], posf[1], posf[2], posf[3], lhtA, lhtB, lhtC, lhtD, 0.25f * posSum);
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
