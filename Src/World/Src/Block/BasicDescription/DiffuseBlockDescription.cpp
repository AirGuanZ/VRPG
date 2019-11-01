#include <VRPG/World/Block/BasicDescription/DiffuseBoxDescription.h>

VRPG_WORLD_BEGIN

DiffuseBoxDescription::DiffuseBoxDescription(
    std::string name,
    std::shared_ptr<const DiffuseBlockEffect> effect, int textureIndexInEffect[],
    BlockBrightness emission)
{
    name_ = std::move(name);

    effect_ = std::move(effect);
    for(int i = 0; i < 6; ++i)
        textureIndexInEffect_[i] = textureIndexInEffect[i];

    isLightSource_ = emission != BLOCK_BRIGHTNESS_MIN;
    emission_ = emission;
}

const char *DiffuseBoxDescription::GetName() const
{
    return name_.c_str();
}

bool DiffuseBoxDescription::IsFullOpaque(Direction direction) const noexcept
{
    return true;
}

bool DiffuseBoxDescription::IsVisible() const noexcept
{
    return true;
}

bool DiffuseBoxDescription::IsLightSource() const noexcept
{
    return isLightSource_;
}

BlockBrightness DiffuseBoxDescription::LightAttenuation() const noexcept
{
    return BLOCK_BRIGHTNESS_MAX;
}

BlockBrightness DiffuseBoxDescription::InitialBrightness() const noexcept
{
    return emission_;
}

void DiffuseBoxDescription::AddBlockModel(
    PartialSectionModelBuilderSet &modelBuilders,
    const Vec3i &blockPosition,
    const BlockDescription *neighborBlocks[3][3][3],
    const BlockBrightness neighborBrightness[3][3][3],
    const BlockOrientation neighborOrientations[3][3][3]) const
{
    // IMPROVE: 没考虑本方块的旋转

    auto builder = modelBuilders.GetBuilderByEffect(effect_.get());
    Vec3 positionBase = blockPosition.map([](int i) { return float(i); });

    auto isFaceVisible = [&](int neighborX, int neighborY, int neighborZ, Direction neighborDir)
    {
        Direction direction = neighborOrientations[neighborX][neighborY][neighborZ].RotatedToOrigin(neighborDir);
        return !neighborBlocks[neighborX][neighborY][neighborZ]->IsFullOpaque(direction);
    };

    // 计算法线为+x/-x的顶点的亮度
    auto vertexAO_x = [&](int x, int y, int z)
    {
        return SIDE_VERTEX_BRIGHTNESS_RATIO * ComputeVertexBrightness(
            neighborBrightness[x][1][1],
            neighborBrightness[x][1 + y][1],
            neighborBrightness[x][1][1 + z],
            neighborBrightness[x][1 + y][1 + z]);
    };

    // 计算法线为+y/-y的顶点的亮度
    auto vertexAO_y = [&](int x, int y, int z)
    {
        float ratio = y > 1 ? 1.0f : SIDE_VERTEX_BRIGHTNESS_RATIO;
        return ratio * ComputeVertexBrightness(
            neighborBrightness[1][y][1],
            neighborBrightness[1 + x][y][1],
            neighborBrightness[1][y][1 + z],
            neighborBrightness[1 + x][y][1 + z]);
    };

    // 计算法线为+z/-z的顶点的亮度
    auto vertexAO_z = [&](int x, int y, int z)
    {
        return SIDE_VERTEX_BRIGHTNESS_RATIO * ComputeVertexBrightness(
            neighborBrightness[1][1][z],
            neighborBrightness[1 + x][1][z],
            neighborBrightness[1][1 + y][z],
            neighborBrightness[1 + x][1 + y][z]);
    };

    auto addFace = [&](
        const Vec3 &posA, const Vec3 &posB, const Vec3 &posC, const Vec3 &posD,
        const Vec4 &lhtA, const Vec4 &lhtB, const Vec4 &lhtC, const Vec4 &lhtD,
        uint32_t textureIndexInEffect)
    {
        Vec3 posE = 0.25f * (posA + posB + posC + posD);
        Vec4 lhtE = 0.25f * (lhtA + lhtB + lhtC + lhtD);

        VertexIndex vertexCount = VertexIndex(builder->GetVertexCount());

        builder->AddVertex({ posA, Vec2(0, 1),       textureIndexInEffect, lhtA });
        builder->AddVertex({ posB, Vec2(0, 0),       textureIndexInEffect, lhtB });
        builder->AddVertex({ posC, Vec2(1, 0),       textureIndexInEffect, lhtC });
        builder->AddVertex({ posD, Vec2(1, 1),       textureIndexInEffect, lhtD });
        builder->AddVertex({ posE, Vec2(0.5f, 0.5f), textureIndexInEffect, lhtE });

        builder->AddIndexedTriangle(vertexCount + 0, vertexCount + 1, vertexCount + 4);
        builder->AddIndexedTriangle(vertexCount + 1, vertexCount + 2, vertexCount + 4);
        builder->AddIndexedTriangle(vertexCount + 2, vertexCount + 3, vertexCount + 4);
        builder->AddIndexedTriangle(vertexCount + 3, vertexCount + 0, vertexCount + 4);
    };

    // +x
    if(isFaceVisible(2, 1, 1, NegativeX))
    {
        Vec3 posA = positionBase + Vec3(1, 0, 0);
        Vec3 posB = positionBase + Vec3(1, 1, 0);
        Vec3 posC = positionBase + Vec3(1, 1, 1);
        Vec3 posD = positionBase + Vec3(1, 0, 1);

        Vec4 lightA = vertexAO_x(2, -1, -1);
        Vec4 lightB = vertexAO_x(2, 1, -1);
        Vec4 lightC = vertexAO_x(2, 1, 1);
        Vec4 lightD = vertexAO_x(2, -1, 1);

        addFace(posA, posB, posC, posD, lightA, lightB, lightC, lightD, textureIndexInEffect_[PositiveX]);
    }

    // -x
    if(isFaceVisible(0, 1, 1, PositiveX))
    {
        Vec3 posA = positionBase + Vec3(0, 0, 1);
        Vec3 posB = positionBase + Vec3(0, 1, 1);
        Vec3 posC = positionBase + Vec3(0, 1, 0);
        Vec3 posD = positionBase + Vec3(0, 0, 0);

        Vec4 lightA = vertexAO_x(0, -1, 1);
        Vec4 lightB = vertexAO_x(0, 1, 1);
        Vec4 lightC = vertexAO_x(0, 1, -1);
        Vec4 lightD = vertexAO_x(0, -1, -1);

        addFace(posA, posB, posC, posD, lightA, lightB, lightC, lightD, textureIndexInEffect_[NegativeX]);
    }

    // +y
    if(isFaceVisible(1, 2, 1, NegativeY))
    {
        Vec3 posA = positionBase + Vec3(0, 1, 0);
        Vec3 posB = positionBase + Vec3(0, 1, 1);
        Vec3 posC = positionBase + Vec3(1, 1, 1);
        Vec3 posD = positionBase + Vec3(1, 1, 0);

        Vec4 lightA = vertexAO_y(-1, 2, -1);
        Vec4 lightB = vertexAO_y(-1, 2, 1);
        Vec4 lightC = vertexAO_y(1, 2, 1);
        Vec4 lightD = vertexAO_y(1, 2, -1);

        addFace(posA, posB, posC, posD, lightA, lightB, lightC, lightD, textureIndexInEffect_[PositiveY]);
    }

    // -y
    if(isFaceVisible(1, 0, 1, PositiveY))
    {
        Vec3 posA = positionBase + Vec3(1, 0, 0);
        Vec3 posB = positionBase + Vec3(1, 0, 1);
        Vec3 posC = positionBase + Vec3(0, 0, 1);
        Vec3 posD = positionBase + Vec3(0, 0, 0);

        Vec4 lightA = vertexAO_y(1, 0, -1);
        Vec4 lightB = vertexAO_y(1, 0, 1);
        Vec4 lightC = vertexAO_y(-1, 0, 1);
        Vec4 lightD = vertexAO_y(-1, 0, -1);

        addFace(posA, posB, posC, posD, lightA, lightB, lightC, lightD, textureIndexInEffect_[NegativeY]);
    }

    // +z
    if(isFaceVisible(1, 1, 2, NegativeZ))
    {
        Vec3 posA = positionBase + Vec3(1, 0, 1);
        Vec3 posB = positionBase + Vec3(1, 1, 1);
        Vec3 posC = positionBase + Vec3(0, 1, 1);
        Vec3 posD = positionBase + Vec3(0, 0, 1);

        Vec4 lightA = vertexAO_z(1, -1, 2);
        Vec4 lightB = vertexAO_z(1, 1, 2);
        Vec4 lightC = vertexAO_z(-1, 1, 2);
        Vec4 lightD = vertexAO_z(-1, -1, 2);

        addFace(posA, posB, posC, posD, lightA, lightB, lightC, lightD, textureIndexInEffect_[PositiveZ]);
    }

    // -z
    if(isFaceVisible(1, 1, 0, PositiveZ))
    {
        Vec3 posA = positionBase + Vec3(0, 0, 0);
        Vec3 posB = positionBase + Vec3(0, 1, 0);
        Vec3 posC = positionBase + Vec3(1, 1, 0);
        Vec3 posD = positionBase + Vec3(1, 0, 0);

        Vec4 lightA = vertexAO_z(-1, -1, 0);
        Vec4 lightB = vertexAO_z(-1, 1, 0);
        Vec4 lightC = vertexAO_z(1, 1, 0);
        Vec4 lightD = vertexAO_z(1, -1, 0);

        addFace(posA, posB, posC, posD, lightA, lightB, lightC, lightD, textureIndexInEffect_[NegativeZ]);
    }
}

VRPG_WORLD_END
