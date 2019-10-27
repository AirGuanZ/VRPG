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
    agz::misc::span<std::unique_ptr<PartialSectionModelBuilder>> modelBuilders,
    const Vec3i &blockPosition,
    const BlockDescription *neighboringBlocks[3][3][3],
    BlockBrightness neighboringBrightness[3][3][3]) const
{
    BlockEffectID effectID = effect_->GetBlockEffectID();
    auto builder = dynamic_cast<DefaultBlockEffect::ModelBuilder*>(modelBuilders[effectID].get());
    assert(builder);

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

        builder->AddTriangle({ posA, lightA }, { posB, lightB }, { posC, lightC });
        builder->AddTriangle({ posA, lightA }, { posC, lightC }, { posD, lightD });
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

        builder->AddTriangle({ posA, lightA }, { posB, lightB }, { posC, lightC });
        builder->AddTriangle({ posA, lightA }, { posC, lightC }, { posD, lightD });
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

        builder->AddTriangle({ posA, lightA }, { posB, lightB }, { posC, lightC });
        builder->AddTriangle({ posA, lightA }, { posC, lightC }, { posD, lightD });
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

        builder->AddTriangle({ posA, lightA }, { posB, lightB }, { posC, lightC });
        builder->AddTriangle({ posA, lightA }, { posC, lightC }, { posD, lightD });
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

        builder->AddTriangle({ posA, lightA }, { posB, lightB }, { posC, lightC });
        builder->AddTriangle({ posA, lightA }, { posC, lightC }, { posD, lightD });
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

        builder->AddTriangle({ posA, lightA }, { posB, lightB }, { posC, lightC });
        builder->AddTriangle({ posA, lightA }, { posC, lightC }, { posD, lightD });
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
