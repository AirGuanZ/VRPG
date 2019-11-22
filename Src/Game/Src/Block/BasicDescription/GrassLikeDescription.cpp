#include <VRPG/Game/Block/BasicCollision/BoxCollision.h>
#include <VRPG/Game/Block/BasicDescription/GrassLikeDescription.h>

VRPG_GAME_BEGIN

GrassLikeDescription::GrassLikeDescription(
    std::string name,
    std::shared_ptr<const GrassLikeEffect> effect, int textureIndexInEffect[2])
    : name_(std::move(name)), effect_(std::move(effect)),
      textureIndexInEffect_{ textureIndexInEffect[0], textureIndexInEffect[1] }
{
    
}

const char *GrassLikeDescription::GetName() const
{
    return name_.c_str();
}

FaceVisibilityType GrassLikeDescription::GetFaceVisibility(Direction direction) const noexcept
{
    return FaceVisibilityType::Nonbox;
}

bool GrassLikeDescription::IsReplacableByLiquid() const noexcept
{
    return true;
}

bool GrassLikeDescription::IsFullOpaque() const noexcept
{
    return false;
}

bool GrassLikeDescription::IsLightSource() const noexcept
{
    return false;
}

BlockBrightness GrassLikeDescription::LightAttenuation() const noexcept
{
    return BlockBrightness{ 1, 1, 1, 1 };
}

BlockBrightness GrassLikeDescription::InitialBrightness() const noexcept
{
    return BLOCK_BRIGHTNESS_MIN;
}

void GrassLikeDescription::AddBlockModel(
    ModelBuilderSet &modelBuilders,
    const Vec3i &blockPosition,
    const BlockNeighborhood blocks) const
{
    auto builder = modelBuilders.GetBuilderByEffect(effect_.get());

    auto addFace =
        [&,
         positionBase = blockPosition.map([](int i) { return float(i); }),
         light = ComputeVertexBrightness(blocks[1][1][1].brightness)]
        (const Vec3 &posA, const Vec3 &posB, const Vec3 &posC, const Vec3 &posD,
         int textureIndexInEffect)
    {
        VertexIndex vertexCount = VertexIndex(builder->GetVertexCount());
        Vec3 normal = cross(posB - posA, posC - posB).normalize();

        builder->AddVertex({
            posA + positionBase, { 0, 1 }, normal, light,
            static_cast<uint32_t>(textureIndexInEffect) });
        builder->AddVertex({
            posB + positionBase, { 0, 0 }, normal, light,
            static_cast<uint32_t>(textureIndexInEffect) });
        builder->AddVertex({
            posC + positionBase, { 1, 0 }, normal, light,
            static_cast<uint32_t>(textureIndexInEffect) });
        builder->AddVertex({
            posD + positionBase, { 1, 1 }, normal, light,
            static_cast<uint32_t>(textureIndexInEffect) });

        builder->AddVertex({
    posA + positionBase, { 0, 1 }, -normal, light,
            static_cast<uint32_t>(textureIndexInEffect) });
        builder->AddVertex({
    posB + positionBase, { 0, 0 }, -normal, light,
            static_cast<uint32_t>(textureIndexInEffect) });
        builder->AddVertex({
    posC + positionBase, { 1, 0 }, -normal, light,
            static_cast<uint32_t>(textureIndexInEffect) });
        builder->AddVertex({
            posD + positionBase, { 1, 1 }, -normal, light,
            static_cast<uint32_t>(textureIndexInEffect) });

        builder->AddIndexedTriangle(vertexCount + 0, vertexCount + 1, vertexCount + 2);
        builder->AddIndexedTriangle(vertexCount + 0, vertexCount + 2, vertexCount + 3);

        builder->AddIndexedTriangle(vertexCount + 7, vertexCount + 6, vertexCount + 5);
        builder->AddIndexedTriangle(vertexCount + 7, vertexCount + 5, vertexCount + 4);
    };

    addFace({ 0, 0, 0 }, { 0, 1, 0 }, { 1, 1, 1 }, { 1, 0, 1 }, textureIndexInEffect_[0]);
    addFace({ 0, 0, 1 }, { 0, 1, 1 }, { 1, 1, 0 }, { 1, 0, 0 }, textureIndexInEffect_[1]);
}

const BlockCollision *GrassLikeDescription::GetCollision() const noexcept
{
    static const BoxBlockCollision ret(false);
    return &ret;
}

VRPG_GAME_END
