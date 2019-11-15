#include <agz/utility/image.h>

#include <VRPG/Game/Block/BasicDescription/DiffuseHollowBoxDescription.h>
#include <VRPG/Game/Block/BasicDescription/DiffuseSolidBoxDescription.h>
#include <VRPG/Game/Block/BasicDescription/TransparentBoxDescription.h>
#include <VRPG/Game/Block/BasicDescription/TransparentLiquidDescription.h>
#include <VRPG/Game/Block/BasicEffect/DiffuseHollowBlockEffect.h>
#include <VRPG/Game/Block/BasicEffect/DiffuseSolidBlockEffect.h>
#include <VRPG/Game/Block/BasicEffect/TransparentBlockEffect.h>
#include <VRPG/Game/Block/BuiltinBlock/BuiltinBlock.h>
#include <VRPG/Game/Config/GlobalConfig.h>

VRPG_GAME_BEGIN

namespace
{
    agz::math::tensor_t<Vec4, 2> LoadSolidTextureFrom(const std::string &filename)
    {
        return agz::img::load_rgb_from_file(filename).map(
        [](const agz::math::color3b &c)
        {
            return Vec4(
                std::pow(c.r / 255.0f, 2.2f),
                std::pow(c.g / 255.0f, 2.2f),
                std::pow(c.b / 255.0f, 2.2f),
                1);
        });
    }

    agz::math::tensor_t<Vec4, 2> LoadHollowTextureFrom(const std::string &filename)
    {
        auto original = agz::img::load_rgba_from_file(filename).map(
            [](const agz::math::color4b &c)
        {
            return Vec4(
                std::pow(c.r / 255.0f, 2.2f),
                std::pow(c.g / 255.0f, 2.2f),
                std::pow(c.b / 255.0f, 2.2f),
                c.a / 255.0f);
        });

        // 计算非透明像素的平均颜色

        int count = 0; Vec3 sum;
        (void)original.map([&](const Vec4 &c)
        {
            if(c.w > 0.5f)
            {
                sum += c.xyz();
                ++count;
            }
            return c;
        });
        Vec3 avg = count == 0 ? Vec3() : 1.0f / count * sum;

        // 利用avg填充透明像素的颜色，但不改变其alpha
        // 这样可以改善在mipmap中透明像素对颜色的影响

        return original.map([&](const Vec4 &c)
        {
            return c.w > 0.5f ? c : Vec4(avg.x, avg.y, avg.z, c.w);
        });
    }

    agz::math::tensor_t<Vec4, 2> LoadTransparentTextureFrom(const std::string &filename)
    {
        return agz::img::load_rgba_from_file(filename).map(
            [](const agz::math::color4b &c)
        {
            return Vec4(
                std::pow(c.r / 255.0f, 2.2f),
                std::pow(c.g / 255.0f, 2.2f),
                std::pow(c.b / 255.0f, 2.2f),
                c.a / 255.0f);
        });
    }
}

void BuiltinBlockTypeManager::RegisterBuiltinBlockTypes()
{
    auto &descMgr = BlockDescriptionManager::GetInstance();

    DiffuseHollowBlockEffectGenerator diffuseHollowBlockEffectGenerator(32, 64);
    DiffuseSolidBlockEffectGenerator diffuseSolidEffectGenerator(32, 64);
    TransparentBlockEffectGenerator transparentBlockEffectGenerator(32);

    {
        auto effect = diffuseSolidEffectGenerator.GetEffectWithTextureSpaces(1);
        int textureIndex = diffuseSolidEffectGenerator.AddTexture(
            LoadSolidTextureFrom(GLOBAL_CONFIG.ASSET_PATH["BuiltinBlock"]["Stone"]["Texture"]).raw_data());
        int textureIndices[] = { textureIndex, textureIndex, textureIndex, textureIndex, textureIndex, textureIndex };
        
        auto stoneDesc = std::make_shared<DiffuseSolidBoxDescription>(
            "stone", effect, textureIndices, BLOCK_BRIGHTNESS_MIN);
        descMgr.RegisterBlockDescription(stoneDesc);
        info_[int(BuiltinBlockType::Stone)].desc = stoneDesc;
    }
    
    {
        auto effect = diffuseSolidEffectGenerator.GetEffectWithTextureSpaces(1);
        int textureIndex = diffuseSolidEffectGenerator.AddTexture(
            LoadSolidTextureFrom(GLOBAL_CONFIG.ASSET_PATH["BuiltinBlock"]["Soil"]["Texture"]).raw_data());
        int textureIndices[] = { textureIndex, textureIndex, textureIndex, textureIndex, textureIndex, textureIndex };
        
        auto soilDesc = std::make_shared<DiffuseSolidBoxDescription>(
            "soil", effect, textureIndices, BLOCK_BRIGHTNESS_MIN);
        descMgr.RegisterBlockDescription(soilDesc);
        info_[int(BuiltinBlockType::Soil)].desc = soilDesc;
    }
    
    {
        auto effect = diffuseSolidEffectGenerator.GetEffectWithTextureSpaces(3);
        int topIndex = diffuseSolidEffectGenerator.AddTexture(
            LoadSolidTextureFrom(GLOBAL_CONFIG.ASSET_PATH["BuiltinBlock"]["Lawn"]["TopTexture"]).raw_data());
        int sideIndex = diffuseSolidEffectGenerator.AddTexture(
            LoadSolidTextureFrom(GLOBAL_CONFIG.ASSET_PATH["BuiltinBlock"]["Lawn"]["SideTexture"]).raw_data());
        int bottomIndex = diffuseSolidEffectGenerator.AddTexture(
            LoadSolidTextureFrom(GLOBAL_CONFIG.ASSET_PATH["BuiltinBlock"]["Lawn"]["BottomTexture"]).raw_data());
        
        int textureIndices[6];
        textureIndices[PositiveX] = sideIndex;
        textureIndices[PositiveY] = topIndex;
        textureIndices[PositiveZ] = sideIndex;
        textureIndices[NegativeX] = sideIndex;
        textureIndices[NegativeY] = bottomIndex;
        textureIndices[NegativeZ] = sideIndex;
        
        auto lawnDesc = std::make_shared<DiffuseSolidBoxDescription>(
            "lawn", effect, textureIndices, BLOCK_BRIGHTNESS_MIN);
        descMgr.RegisterBlockDescription(lawnDesc);
        info_[int(BuiltinBlockType::Lawn)].desc = lawnDesc;
    }

    {
        auto effect = diffuseSolidEffectGenerator.GetEffectWithTextureSpaces(3);
        int topIndex = diffuseSolidEffectGenerator.AddTexture(
            LoadSolidTextureFrom(GLOBAL_CONFIG.ASSET_PATH["BuiltinBlock"]["Log"]["TopTexture"]).raw_data());
        int sideIndex = diffuseSolidEffectGenerator.AddTexture(
            LoadSolidTextureFrom(GLOBAL_CONFIG.ASSET_PATH["BuiltinBlock"]["Log"]["SideTexture"]).raw_data());
        int bottomIndex = diffuseSolidEffectGenerator.AddTexture(
            LoadSolidTextureFrom(GLOBAL_CONFIG.ASSET_PATH["BuiltinBlock"]["Log"]["BottomTexture"]).raw_data());

        int textureIndices[6];
        textureIndices[PositiveX] = sideIndex;
        textureIndices[NegativeX] = sideIndex;
        textureIndices[PositiveZ] = sideIndex;
        textureIndices[NegativeZ] = sideIndex;
        textureIndices[PositiveY] = topIndex;
        textureIndices[NegativeY] = bottomIndex;

        auto logDesc = std::make_shared<DiffuseSolidBoxDescription>(
            "log", effect, textureIndices, BLOCK_BRIGHTNESS_MIN);
        descMgr.RegisterBlockDescription(logDesc);
        info_[int(BuiltinBlockType::Log)].desc = logDesc;
    }

    {
        auto effect = diffuseSolidEffectGenerator.GetEffectWithTextureSpaces(1);
        int textureIndex = diffuseSolidEffectGenerator.AddTexture(
            LoadSolidTextureFrom(GLOBAL_CONFIG.ASSET_PATH["BuiltinBlock"]["GlowStone"]["Texture"]).raw_data());
        int textureIndices[] = { textureIndex, textureIndex, textureIndex, textureIndex, textureIndex, textureIndex };

        auto glowStoneDesc = std::make_shared<DiffuseSolidBoxDescription>(
            "glow stone", effect, textureIndices, BlockBrightness{ 15, 15, 15, 0 });
        descMgr.RegisterBlockDescription(glowStoneDesc);
        info_[int(BuiltinBlockType::GlowStone)].desc = glowStoneDesc;
    }

    {
        auto effect = diffuseHollowBlockEffectGenerator.GetEffectWithTextureSpaces(1);
        int textureIndex = diffuseHollowBlockEffectGenerator.AddTexture(
            LoadHollowTextureFrom(GLOBAL_CONFIG.ASSET_PATH["BuiltinBlock"]["Leaf"]["Texture"]).raw_data());
        int textureIndices[] = { textureIndex, textureIndex, textureIndex, textureIndex, textureIndex, textureIndex };

        auto leafDesc = std::make_shared<DiffuseHollowBoxDescription>(
            "leaf", effect, textureIndices, BLOCK_BRIGHTNESS_MIN, BlockBrightness{ 1, 1, 1, 1 });
        descMgr.RegisterBlockDescription(leafDesc);
        info_[int(BuiltinBlockType::Leaf)].desc = leafDesc;
    }

    {
        auto effect = transparentBlockEffectGenerator.GetEffectWithTextureSpaces();
        int textureIndex = transparentBlockEffectGenerator.AddTexture(
            LoadTransparentTextureFrom(GLOBAL_CONFIG.ASSET_PATH["BuiltinBlock"]["WhiteGlass"]["Texture"]).raw_data());
        int textureIndices[] = { textureIndex, textureIndex, textureIndex, textureIndex, textureIndex, textureIndex };

        auto whiteGlassDesc = std::make_shared<TransparentBoxDescription>(
            "white glass", effect, textureIndices, BlockBrightness{ 1, 1, 1, 1 });
        descMgr.RegisterBlockDescription(whiteGlassDesc);
        info_[int(BuiltinBlockType::WhiteGlass)].desc = whiteGlassDesc;
    }

    {
        auto effect = transparentBlockEffectGenerator.GetEffectWithTextureSpaces();
        int textureIndex = transparentBlockEffectGenerator.AddTexture(
            LoadTransparentTextureFrom(GLOBAL_CONFIG.ASSET_PATH["BuiltinBlock"]["RedGlass"]["Texture"]).raw_data());
        int textureIndices[] = { textureIndex, textureIndex, textureIndex, textureIndex, textureIndex, textureIndex };

        auto whiteGlassDesc = std::make_shared<TransparentBoxDescription>(
            "red glass", effect, textureIndices, BlockBrightness{ 1, 1, 1, 1 });
        descMgr.RegisterBlockDescription(whiteGlassDesc);
        info_[int(BuiltinBlockType::RedGlass)].desc = whiteGlassDesc;
    }

    {
        auto effect = transparentBlockEffectGenerator.GetEffectWithTextureSpaces();
        int textureIndex = transparentBlockEffectGenerator.AddTexture(
            LoadTransparentTextureFrom(GLOBAL_CONFIG.ASSET_PATH["BuiltinBlock"]["Water"]["Texture"]).raw_data());

        LiquidDescription waterLiquid;
        waterLiquid.isLiquid = true;
        auto waterDesc = std::make_shared<TransparentLiquidDescription>(
            "water", waterLiquid, effect, textureIndex, BlockBrightness{ 1, 1, 1, 1 });
        descMgr.RegisterBlockDescription(waterDesc);
        info_[int(BuiltinBlockType::Water)].desc = waterDesc;
    }

    diffuseHollowBlockEffectGenerator.Done();
    diffuseSolidEffectGenerator.Done();
    transparentBlockEffectGenerator.Done();
}

VRPG_GAME_END
