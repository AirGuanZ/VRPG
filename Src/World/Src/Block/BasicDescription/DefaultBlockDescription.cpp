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
    const BlockDescription *neighborBlocks[3][3][3],
    const BlockBrightness neighborBrightness[3][3][3],
    const BlockOrientation neighborOrientations[3][3][3]) const
{
    auto builder = modelBuilders.GetBuilderByEffect(effect_);
    Vec3 blockPositionf = blockPosition.map([](int i) { return float(i); });

    auto vertexAO_x = [&](int x, int y, int z)
    {
        return SIDE_VERTEX_BRIGHTNESS_RATIO * ComputeVertexBrightness(
            neighborBrightness[x][1    ][1    ],
            neighborBrightness[x][1 + y][1    ],
            neighborBrightness[x][1    ][1 + z],
            neighborBrightness[x][1 + y][1 + z]);
    };

    auto vertexAO_y = [&](int x, int y, int z)
    {
        float ratio = y > 1 ? 1.0f : SIDE_VERTEX_BRIGHTNESS_RATIO;
        return ratio * ComputeVertexBrightness(
            neighborBrightness[1    ][y][1    ],
            neighborBrightness[1 + x][y][1    ],
            neighborBrightness[1    ][y][1 + z],
            neighborBrightness[1 + x][y][1 + z]);
    };

    auto vertexAO_z = [&](int x, int y, int z)
    {
        return SIDE_VERTEX_BRIGHTNESS_RATIO * ComputeVertexBrightness(
            neighborBrightness[1    ][1    ][z],
            neighborBrightness[1 + x][1    ][z],
            neighborBrightness[1    ][1 + y][z],
            neighborBrightness[1 + x][1 + y][z]);
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

    auto isFaceVisible = [&](int neighborX, int neighborY, int neighborZ, Direction neighborDir)
    {
        Direction direction = neighborOrientations[neighborX][neighborY][neighborZ].RotatedToOrigin(neighborDir);
        return !neighborBlocks[neighborX][neighborY][neighborZ]->IsFullOpaque(direction);
    };

    // +x
    if(isFaceVisible(2, 1, 1, NegativeX))
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
    if(isFaceVisible(0, 1, 1, PositiveX))
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
    if(isFaceVisible(1, 2, 1, NegativeY))
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
    if(isFaceVisible(1, 0, 1, PositiveY))
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
    if(isFaceVisible(1, 1, 2, NegativeZ))
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
    if(isFaceVisible(1, 1, 0, PositiveZ))
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
