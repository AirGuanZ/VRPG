#include <VRPG/Game/Block/BasicEffect/DefaultBlockEffect.h>
#include <VRPG/Game/Block/BlockEffect.h>

VRPG_GAME_BEGIN

BlockEffectManager::BlockEffectManager()
{
    RegisterBlockEffect(std::make_shared<DefaultBlockEffect>());
}

BlockEffectID BlockEffectManager::RegisterBlockEffect(std::shared_ptr<BlockEffect> effect)
{
    assert(blockEffects_.size() < (std::numeric_limits<BlockEffectID>::max)());

    if(auto it = name2Effect_.find(effect->GetName()); it != name2Effect_.end())
    {
        if(it->second != effect)
            throw VRPGGameException("repeated block effect name: " + std::string(effect->GetName()));
        return effect->GetBlockEffectID();
    }

    BlockEffectID id = BlockEffectID(blockEffects_.size());
    effect->SetBlockEffectID(id);
    spdlog::info("register block effect (name = {}, id = {})", effect->GetName(), id);

    rawBlockEffects_.push_back(effect.get());
    name2Effect_[std::string(effect->GetName())] = effect;
    blockEffects_.push_back(std::move(effect));
    return id;
}

void BlockEffectManager::Clear()
{
    blockEffects_.clear();
    name2Effect_.clear();
    rawBlockEffects_.clear();

    auto defaultEffect = std::make_shared<DefaultBlockEffect>();
    defaultEffect->SetBlockEffectID(BLOCK_EFFECT_ID_DEFAULT);

    rawBlockEffects_.push_back(defaultEffect.get());
    name2Effect_[std::string(defaultEffect->GetName())] = defaultEffect;
    blockEffects_.push_back(std::move(defaultEffect));
}

VRPG_GAME_END
