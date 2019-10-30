#include <VRPG/World/Block/BasicDescription/DefaultBlockDescription.h>
#include <VRPG/World/Block/BasicEffect/DefaultBlockEffect.h>

VRPG_WORLD_BEGIN

DefaultBlockDescription::DefaultBlockDescription(const DefaultBlockEffect *effect) noexcept
    : effect_(effect)
{
    
}

bool DefaultBlockDescription::IsFullOpaque(Direction direction) const noexcept
{
    return true;
}

bool DefaultBlockDescription::IsVisible() const noexcept
{
    return true;
}

void DefaultBlockDescription::AddBlockModel(
    PartialSectionModelBuilderSet &modelBuilders,
    const Vec3i &blockPosition,
    const BlockDescription *neighboringBlocks[3][3][3],
    BlockBrightness neighboringBrightness[3][3][3]) const
{
    auto builder = modelBuilders.GetBuilderByEffect(effect_);
    Vec3 blockPositionf = blockPosition.map([](int i) { return float(i); });

    auto vertexMean = [&](
        BlockBrightness a,
        BlockBrightness b,
        BlockBrightness c,
        BlockBrightness d)
    {
        return BlockAO(
            BlockBrightnessToFloat(a),
            BlockBrightnessToFloat(b),
            BlockBrightnessToFloat(c),
            BlockBrightnessToFloat(d));
    };

    auto vertexAO_x = [&](int x, int y, int z)
    {
        return vertexMean(
            neighboringBrightness[x][1    ][1    ],
            neighboringBrightness[x][1 + y][1    ],
            neighboringBrightness[x][1    ][1 + z],
            neighboringBrightness[x][1 + y][1 + z]);
    };

    auto vertexAO_y = [&](int x, int y, int z)
    {
        return vertexMean(
            neighboringBrightness[1    ][y][1    ],
            neighboringBrightness[1 + x][y][1    ],
            neighboringBrightness[1    ][y][1 + z],
            neighboringBrightness[1 + x][y][1 + z]);
    };

    auto vertexAO_z = [&](int x, int y, int z)
    {
        return vertexMean(
            neighboringBrightness[1    ][1    ][z],
            neighboringBrightness[1 + x][1    ][z],
            neighboringBrightness[1    ][1 + y][z],
            neighboringBrightness[1 + x][1 + y][z]);
    };

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

    // +x
    if(!neighboringBlocks[2][1][1]->IsFullOpaque(NegativeX))
    {
        Vec3 posA = blockPositionf + Vec3(1, 0, 0);
        Vec3 posB = blockPositionf + Vec3(1, 1, 0);
        Vec3 posC = blockPositionf + Vec3(1, 1, 1);
        Vec3 posD = blockPositionf + Vec3(1, 0, 1);

        Vec4 lightA = vertexAO_x(2, -1, -1);
        Vec4 lightB = vertexAO_x(2, 1, -1);
        Vec4 lightC = vertexAO_x(2, 1, 1);
        Vec4 lightD = vertexAO_x(2, -1, 1);

        addFace(posA, posB, posC, posD, lightA, lightB, lightC, lightD);
    }

    // -x
    if(!neighboringBlocks[0][1][1]->IsFullOpaque(PositiveX))
    {
        Vec3 posA = blockPositionf + Vec3(0, 0, 1);
        Vec3 posB = blockPositionf + Vec3(0, 1, 1);
        Vec3 posC = blockPositionf + Vec3(0, 1, 0);
        Vec3 posD = blockPositionf + Vec3(0, 0, 0);

        Vec4 lightA = vertexAO_x(0, -1, 1);
        Vec4 lightB = vertexAO_x(0, 1, 1);
        Vec4 lightC = vertexAO_x(0, 1, -1);
        Vec4 lightD = vertexAO_x(0, -1, -1);

        addFace(posA, posB, posC, posD, lightA, lightB, lightC, lightD);
    }

    // +y
    if(!neighboringBlocks[1][2][1]->IsFullOpaque(NegativeY))
    {
        Vec3 posA = blockPositionf + Vec3(0, 1, 0);
        Vec3 posB = blockPositionf + Vec3(0, 1, 1);
        Vec3 posC = blockPositionf + Vec3(1, 1, 1);
        Vec3 posD = blockPositionf + Vec3(1, 1, 0);

        Vec4 lightA = vertexAO_y(-1, 2, -1);
        Vec4 lightB = vertexAO_y(-1, 2, 1);
        Vec4 lightC = vertexAO_y(1, 2, 1);
        Vec4 lightD = vertexAO_y(1, 2, -1);

        addFace(posA, posB, posC, posD, lightA, lightB, lightC, lightD);
    }

    // -y
    if(!neighboringBlocks[1][0][1]->IsFullOpaque(PositiveY))
    {
        Vec3 posA = blockPositionf + Vec3(1, 0, 0);
        Vec3 posB = blockPositionf + Vec3(1, 0, 1);
        Vec3 posC = blockPositionf + Vec3(0, 0, 1);
        Vec3 posD = blockPositionf + Vec3(0, 0, 0);

        Vec4 lightA = vertexAO_y(1, 0, -1);
        Vec4 lightB = vertexAO_y(1, 0, 1);
        Vec4 lightC = vertexAO_y(-1, 0, 1);
        Vec4 lightD = vertexAO_y(-1, 0, -1);

        addFace(posA, posB, posC, posD, lightA, lightB, lightC, lightD);
    }

    // +z
    if(!neighboringBlocks[1][1][2]->IsFullOpaque(NegativeZ))
    {
        Vec3 posA = blockPositionf + Vec3(1, 0, 1);
        Vec3 posB = blockPositionf + Vec3(1, 1, 1);
        Vec3 posC = blockPositionf + Vec3(0, 1, 1);
        Vec3 posD = blockPositionf + Vec3(0, 0, 1);

        Vec4 lightA = vertexAO_z(1, -1, 2);
        Vec4 lightB = vertexAO_z(1, 1, 2);
        Vec4 lightC = vertexAO_z(-1, 1, 2);
        Vec4 lightD = vertexAO_z(-1, -1, 2);

        addFace(posA, posB, posC, posD, lightA, lightB, lightC, lightD);
    }

    // -z
    if(!neighboringBlocks[1][1][0]->IsFullOpaque(PositiveZ))
    {
        Vec3 posA = blockPositionf + Vec3(0, 0, 0);
        Vec3 posB = blockPositionf + Vec3(0, 1, 0);
        Vec3 posC = blockPositionf + Vec3(1, 1, 0);
        Vec3 posD = blockPositionf + Vec3(1, 0, 0);

        Vec4 lightA = vertexAO_z(-1, -1, 0);
        Vec4 lightB = vertexAO_z(-1, 1, 0);
        Vec4 lightC = vertexAO_z(1, 1, 0);
        Vec4 lightD = vertexAO_z(1, -1, 0);

        addFace(posA, posB, posC, posD, lightA, lightB, lightC, lightD);
    }
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

VRPG_WORLD_END