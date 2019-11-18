#include <VRPG/Game/Block/BasicCollision/VoidCollision.h>
#include <VRPG/Game/Block/BasicDescription/DefaultBoxDescription.h>
#include <VRPG/Game/Block/BasicEffect/DefaultBlockEffect.h>
#include <VRPG/Game/Block/BlockDescription.h>
#include <VRPG/Game/Block/BlockEffect.h>
#include <VRPG/Game/Block/LiquidDescription.h>

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

        FaceVisibilityType GetFaceVisibility(Direction direction) const noexcept override;

        bool IsVisible() const noexcept override;

        bool IsReplacableByLiquid() const noexcept override;

        bool IsFullOpaque() const noexcept override;

        void AddBlockModel(ModelBuilderSet &, const Vec3i &, const BlockNeighborhood) const override;

        bool IsLightSource() const noexcept override;

        BlockBrightness LightAttenuation() const noexcept override;

        BlockBrightness InitialBrightness() const noexcept override;
    };
}

bool BlockDescription::HasExtraData() const  noexcept
{
    return false;
}

BlockExtraData BlockDescription::CreateExtraData() const
{
    return BlockExtraData();
}

const BlockCollision *BlockDescription::GetCollision() const noexcept
{
    static const VoidBlockCollision ret;
    return &ret;
}

const LiquidDescription *BlockDescription::GetLiquid() const noexcept
{
    static const LiquidDescription ret;
    return &ret;
}

const char *VoidBlockDescription::GetName() const
{
    return "void";
}

FaceVisibilityType VoidBlockDescription::GetFaceVisibility(Direction direction) const noexcept
{
    return FaceVisibilityType::Nonbox;
}

bool VoidBlockDescription::IsVisible() const noexcept
{
    return false;
}

bool VoidBlockDescription::IsReplacableByLiquid() const noexcept
{
    return true;
}

bool VoidBlockDescription::IsFullOpaque() const noexcept
{
    return false;
}

void VoidBlockDescription::AddBlockModel(ModelBuilderSet &, const Vec3i &, const BlockNeighborhood) const
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

BlockDescManager::BlockDescManager()
{
    auto voidDesc = std::make_shared<VoidBlockDescription>();
    RegisterBlockDescription(std::move(voidDesc));

    auto defaultEffect = BlockEffectManager::GetInstance().GetSharedBlockEffect(BLOCK_EFFECT_ID_DEFAULT);
    auto defaultDesc = std::make_shared<DefaultBlockDescription>(
        std::dynamic_pointer_cast<const DefaultBlockEffect>(defaultEffect));
    defaultDesc->SetBlockID(BLOCK_ID_DEFAULT);
    RegisterBlockDescription(std::move(defaultDesc));
}

BlockID BlockDescManager::RegisterBlockDescription(std::shared_ptr<BlockDescription> desc)
{
    assert(blockDescriptions_.size() < (std::numeric_limits<BlockID>::max)());

    if(auto it = name2Desc_.find(desc->GetName()); it != name2Desc_.end())
    {
        if(it->second != desc)
        {
            throw VRPGGameException("repeated block description name: " + std::string(desc->GetName()));
        }
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

void BlockDescManager::Clear()
{
    blockDescriptions_.clear();
    name2Desc_.clear();
    rawBlockDescriptions_.clear();
}

VRPG_GAME_END
