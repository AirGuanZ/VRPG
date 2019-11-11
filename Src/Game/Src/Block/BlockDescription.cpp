#include <VRPG/Game/Block/BasicDescription/DefaultBoxDescription.h>
#include <VRPG/Game/Block/BasicEffect/DefaultBlockEffect.h>
#include <VRPG/Game/Block/BlockDescription.h>
#include <VRPG/Game/Block/BlockEffect.h>
#include <VRPG/Game/Block/LiquidDescription.h>
#include <VRPG/Game/Utility/RayBoxIntersect.h>

VRPG_GAME_BEGIN

namespace
{
    /**
     * @brief 空方块
     */
    class VoidBlockDescription : public BlockDescription
    {
    public:

        const char *GetName() const override;

        FaceVisibilityProperty GetFaceVisibilityProperty(Direction direction) const noexcept override;

        bool IsVisible() const noexcept override;

        bool IsReplacable() const noexcept override;

        bool IsFullOpaque() const noexcept override;

        void AddBlockModel(PartialSectionModelBuilderSet &, const Vec3i &, const BlockNeighborhood) const override;

        bool IsLightSource() const noexcept override;

        BlockBrightness LightAttenuation() const noexcept override;

        BlockBrightness InitialBrightness() const noexcept override;

        bool RayIntersect(const Vec3 &start, const Vec3 &invDir, float minT, float maxT, Direction *pickedFace) const noexcept override;
    };
}

bool BlockDescription::RayIntersect(
    const Vec3 &start, const Vec3 &dir, float minT, float maxT, Direction *pickedFace) const noexcept
{
    return RayIntersectStdBox(start, dir, minT, maxT, pickedFace);
}

bool BlockDescription::HasExtraData() const  noexcept
{
    return false;
}

BlockExtraData BlockDescription::CreateExtraData() const
{
    return BlockExtraData();
}

const LiquidDescription *BlockDescription::GetLiquidDescription() const noexcept
{
    static const LiquidDescription ret;
    return &ret;
}

const char *VoidBlockDescription::GetName() const
{
    return "void";
}

FaceVisibilityProperty VoidBlockDescription::GetFaceVisibilityProperty(Direction direction) const noexcept
{
    return FaceVisibilityProperty::Nonbox;
}

bool VoidBlockDescription::IsVisible() const noexcept
{
    return false;
}

bool VoidBlockDescription::IsReplacable() const noexcept
{
    return true;
}

bool VoidBlockDescription::IsFullOpaque() const noexcept
{
    return false;
}

void VoidBlockDescription::AddBlockModel(PartialSectionModelBuilderSet &, const Vec3i &, const BlockNeighborhood) const
{
    // do nothing
}

bool VoidBlockDescription::IsLightSource() const noexcept
{
    return false;
}

BlockBrightness VoidBlockDescription::LightAttenuation() const noexcept
{
    return { 1, 1, 1, 1 };
}

BlockBrightness VoidBlockDescription::InitialBrightness() const noexcept
{
    return { 0, 0, 0, 0 };
}

bool VoidBlockDescription::RayIntersect(const Vec3 &start, const Vec3 &invDir, float minT, float maxT, Direction *pickedFace) const noexcept
{
    return false;
}

BlockDescriptionManager::BlockDescriptionManager()
{
    auto voidDesc = std::make_shared<VoidBlockDescription>();
    RegisterBlockDescription(std::move(voidDesc));

    auto defaultEffect = BlockEffectManager::GetInstance().GetSharedBlockEffect(BLOCK_EFFECT_ID_DEFAULT);
    auto defaultDesc = std::make_shared<DefaultBlockDescription>(
        std::dynamic_pointer_cast<const DefaultBlockEffect>(defaultEffect));
    defaultDesc->SetBlockID(BLOCK_ID_DEFAULT);
    RegisterBlockDescription(std::move(defaultDesc));
}

BlockID BlockDescriptionManager::RegisterBlockDescription(std::shared_ptr<BlockDescription> desc)
{
    assert(blockDescriptions_.size() < (std::numeric_limits<BlockID>::max)());

    if(auto it = name2Desc_.find(desc->GetName()); it != name2Desc_.end())
    {
        if(it->second != desc)
            throw VRPGWorldException("repeated block description name: " + std::string(desc->GetName()));
        return desc->GetBlockID();
    }

    BlockID id = BlockID(blockDescriptions_.size());
    desc->SetBlockID(id);
    spdlog::info("register block description (name = {}, id = {})", desc->GetName(), id);

    rawBlockDescriptions_.push_back(desc.get());
    name2Desc_[std::string(desc->GetName())] = desc;
    blockDescriptions_.push_back(std::move(desc));
    return id;
}

void BlockDescriptionManager::Clear()
{
    blockDescriptions_.clear();
    name2Desc_.clear();
    rawBlockDescriptions_.clear();

    auto voidDesc = std::make_shared<VoidBlockDescription>();
    voidDesc->SetBlockID(BLOCK_ID_VOID);

    rawBlockDescriptions_.push_back(voidDesc.get());
    name2Desc_[std::string(voidDesc->GetName())] = voidDesc;
    blockDescriptions_.push_back(std::move(voidDesc));

    auto defaultEffect = BlockEffectManager::GetInstance().GetSharedBlockEffect(BLOCK_EFFECT_ID_DEFAULT);
    auto defaultDesc = std::make_shared<DefaultBlockDescription>(
        std::dynamic_pointer_cast<const DefaultBlockEffect>(defaultEffect));
    defaultDesc->SetBlockID(BLOCK_ID_DEFAULT);

    rawBlockDescriptions_.push_back(defaultDesc.get());
    name2Desc_[std::string(defaultDesc->GetName())] = defaultDesc;
    blockDescriptions_.push_back(std::move(defaultDesc));
}

VRPG_GAME_END
