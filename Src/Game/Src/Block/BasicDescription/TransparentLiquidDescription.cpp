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

    auto addFace = [&, blockLight = ComputeVertexBrightness(blocks[1][1][1].brightness)](
        const Vec3 &posA, const Vec3 &posB, const Vec3 &posC, const Vec3 &posD)
    {
        Vec3 posE = 0.25f * (posA + posB + posC + posD);

        VertexIndex vertexCount = VertexIndex(builder->GetVertexCount());
        VertexIndex startIndex = VertexIndex(builder->GetIndexCount());

        // IMPROVE: 没有针对透明液体做光照平滑
        builder->AddVertex({ positionBase + posA, Vec2(0.5f), uint32_t(textureIndexInEffect_), blockLight });
        builder->AddVertex({ positionBase + posB, Vec2(0.5f), uint32_t(textureIndexInEffect_), blockLight });
        builder->AddVertex({ positionBase + posC, Vec2(0.5f), uint32_t(textureIndexInEffect_), blockLight });
        builder->AddVertex({ positionBase + posD, Vec2(0.5f), uint32_t(textureIndexInEffect_), blockLight });
        builder->AddVertex({ positionBase + posE, Vec2(0.5f), uint32_t(textureIndexInEffect_), blockLight });

        builder->AddIndexedTriangle(vertexCount + 0, vertexCount + 1, vertexCount + 4);
        builder->AddIndexedTriangle(vertexCount + 1, vertexCount + 2, vertexCount + 4);
        builder->AddIndexedTriangle(vertexCount + 2, vertexCount + 3, vertexCount + 4);
        builder->AddIndexedTriangle(vertexCount + 3, vertexCount + 0, vertexCount + 4);

        builder->AddFaceIndexRange(positionBase + posE, startIndex);
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

        auto synVertexHeight = [&](int dx, int dz) -> float
        {
            if(!isThisSource && !isUpSame && blocks[1 + dx][1][1].desc->IsVoid() && blocks[1][1][1 + dz].desc->IsVoid())
                return 0;
            return (std::max)((std::max)(blockHeights[1][1], blockHeights[1 + dx][1 + dz]),
                              (std::max)(blockHeights[1 + dx][1], blockHeights[1][1 + dz]));
        };

        vertexHeights[0][0] = synVertexHeight(-1, -1);
        vertexHeights[0][1] = synVertexHeight(-1, +1);
        vertexHeights[1][0] = synVertexHeight(+1, -1);
        vertexHeights[1][1] = synVertexHeight(+1, +1);
        
        /*vertexHeights[0][0] = (std::max)(
            (std::max)(blockHeights[1][1], blockHeights[1][0]),
            (std::max)(blockHeights[0][1], blockHeights[0][0]));
        
        vertexHeights[0][1] = (std::max)(
            (std::max)(blockHeights[0][1], blockHeights[0][2]),
            (std::max)(blockHeights[1][1], blockHeights[1][2]));
        
        vertexHeights[1][0] = (std::max)(
            (std::max)(blockHeights[1][0], blockHeights[1][1]),
            (std::max)(blockHeights[2][0], blockHeights[2][1]));

        vertexHeights[1][1] = (std::max)(
            (std::max)(blockHeights[1][1], blockHeights[1][2]),
            (std::max)(blockHeights[2][1], blockHeights[2][2]));*/
    }

    auto generateFace = [&](Direction normalDirection)
    {
        if(!isFaceVisible(normalDirection))
            return;

        Vec3i pos[4]; Vec3 posf[4];
        GenerateBoxFaceiDynamic(normalDirection, pos);
        for(int i = 0; i < 4; ++i)
        {
            int xi = pos[i].x, zi = pos[i].z;
            posf[i] = { float(xi), pos[i].y * vertexHeights[xi][zi], float(zi) };
        }
        
        addFace(posf[0], posf[1], posf[2], posf[3]);
    };

    generateFace(PositiveX);
    generateFace(NegativeX);
    generateFace(PositiveY);
    generateFace(NegativeY);
    generateFace(PositiveZ);
    generateFace(NegativeZ);

    /*float vertexHeight = 1;
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
    generateFace(NegativeZ);*/
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
