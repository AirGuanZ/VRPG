#include <agz/utility/image.h>

#include <VRPG/World/Block/BasicDescription/DefaultBlockDescription.h>
#include <VRPG/World/Block/BasicDescription/DiffuseHollowBoxDescription.h>
#include <VRPG/World/Block/BasicDescription/DiffuseSolidBoxDescription.h>
#include <VRPG/World/Block/BasicEffect/DefaultBlockEffect.h>
#include <VRPG/World/Block/BasicEffect/DiffuseHollowBlockEffect.h>
#include <VRPG/World/Block/BasicEffect/DiffuseSolidBlockEffect.h>
#include <VRPG/World/Block/BuiltinBlock/BuiltinBlock.h>

VRPG_WORLD_BEGIN

namespace
{
	agz::math::tensor_t<Vec3, 2> LoadRGBTextureFrom(const std::string &filename)
	{
		return agz::img::load_rgb_from_file(filename).map(
        [](const agz::math::color3b &c)
		{
            return Vec3(
                std::pow(c.r / 255.0f, 2.2f),
                std::pow(c.g / 255.0f, 2.2f),
                std::pow(c.b / 255.0f, 2.2f));
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
        original.map([&](const Vec4 &c)
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
}

void BuiltinBlockTypeManager::RegisterBuiltinBlockTypes()
{
	auto &descMgr = BlockDescriptionManager::GetInstance();
	auto &effectMgr = BlockEffectManager::GetInstance();
	
    {
		auto defaultEffect = std::make_shared<DefaultBlockEffect>();
		effectMgr.RegisterBlockEffect(defaultEffect);
		
		auto defaultDesc = std::make_shared<DefaultBlockDescription>(defaultEffect);
		descMgr.RegisterBlockDescription(defaultDesc);
		info_[int(BuiltinBlockType::Default)].desc = defaultDesc;
	}

    DiffuseHollowBlockEffectGenerator diffuseHollowBlockEffectGenerator(32, 64);
    auto diffuseHollowEffect = std::make_shared<DiffuseHollowBlockEffect>();

    DiffuseSolidBlockEffectGenerator diffuseSolidEffectGenerator(32, 64);
	auto diffuseSolidEffect = std::make_shared<DiffuseSolidBlockEffect>();

    auto prepareDiffuseHollowTextureSpace = [&](int textureCount)
    {
        if(!diffuseHollowBlockEffectGenerator.HasEnoughSpaceFor(textureCount))
        {
            diffuseHollowBlockEffectGenerator.InitializeEffect(*diffuseHollowEffect);
            effectMgr.RegisterBlockEffect(diffuseHollowEffect);
            diffuseHollowEffect = std::make_shared<DiffuseHollowBlockEffect>();
        }
    };

	auto prepareDiffuseSolidTextureSpace = [&](int textureCount)
	{	
		if(!diffuseSolidEffectGenerator.HasEnoughSpaceFor(textureCount))
		{
			diffuseSolidEffectGenerator.InitializeEffect(*diffuseSolidEffect);
            effectMgr.RegisterBlockEffect(diffuseSolidEffect);
			diffuseSolidEffect = std::make_shared<DiffuseSolidBlockEffect>();
		}
	};
	
	{
        prepareDiffuseSolidTextureSpace(1);
		int textureIndex = diffuseSolidEffectGenerator.AddTexture(LoadRGBTextureFrom("Asset/World/Texture/BuiltinBlock/Stone.png").raw_data());
		int textureIndices[] = { textureIndex, textureIndex, textureIndex, textureIndex, textureIndex, textureIndex };
		
		auto stoneDesc = std::make_shared<DiffuseSolidBoxDescription>(
			"stone", diffuseSolidEffect, textureIndices, BLOCK_BRIGHTNESS_MIN);
		descMgr.RegisterBlockDescription(stoneDesc);
		info_[int(BuiltinBlockType::Stone)].desc = stoneDesc;
	}
	
	{
        prepareDiffuseSolidTextureSpace(1);
		int textureIndex = diffuseSolidEffectGenerator.AddTexture(LoadRGBTextureFrom("Asset/World/Texture/BuiltinBlock/Soil.png").raw_data());
		int textureIndices[] = { textureIndex, textureIndex, textureIndex, textureIndex, textureIndex, textureIndex };
		
		auto soilDesc = std::make_shared<DiffuseSolidBoxDescription>(
			"soil", diffuseSolidEffect, textureIndices, BLOCK_BRIGHTNESS_MIN);
		descMgr.RegisterBlockDescription(soilDesc);
		info_[int(BuiltinBlockType::Soil)].desc = soilDesc;
	}
	
	{
        prepareDiffuseSolidTextureSpace(3);
		int topIndex    = diffuseSolidEffectGenerator.AddTexture(LoadRGBTextureFrom("Asset/World/Texture/BuiltinBlock/LawnTop.png").raw_data());
		int sideIndex   = diffuseSolidEffectGenerator.AddTexture(LoadRGBTextureFrom("Asset/World/Texture/BuiltinBlock/LawnSide.png").raw_data());
		int bottomIndex = diffuseSolidEffectGenerator.AddTexture(LoadRGBTextureFrom("Asset/World/Texture/BuiltinBlock/LawnBottom.png").raw_data());
		
		int textureIndices[6];
		textureIndices[PositiveX] = sideIndex;
		textureIndices[PositiveY] = topIndex;
		textureIndices[PositiveZ] = sideIndex;
		textureIndices[NegativeX] = sideIndex;
		textureIndices[NegativeY] = bottomIndex;
		textureIndices[NegativeZ] = sideIndex;
		
		auto lawnDesc = std::make_shared<DiffuseSolidBoxDescription>(
			"lawn", diffuseSolidEffect, textureIndices, BLOCK_BRIGHTNESS_MIN);
		descMgr.RegisterBlockDescription(lawnDesc);
		info_[int(BuiltinBlockType::Lawn)].desc = lawnDesc;
	}

    {
        prepareDiffuseSolidTextureSpace(1);
        int textureIndex = diffuseSolidEffectGenerator.AddTexture(LoadRGBTextureFrom("Asset/World/Texture/BuiltinBlock/GlowStone.png").raw_data());
        int textureIndices[] = { textureIndex, textureIndex, textureIndex, textureIndex, textureIndex, textureIndex };

        auto glowStoneDesc = std::make_shared<DiffuseSolidBoxDescription>(
            "glow stone", diffuseSolidEffect, textureIndices, BlockBrightness{ 15, 15, 15, 0 });
        descMgr.RegisterBlockDescription(glowStoneDesc);
        info_[int(BuiltinBlockType::GlowStone)].desc = glowStoneDesc;
    }

	{
        prepareDiffuseHollowTextureSpace(1);
        int textureIndex = diffuseHollowBlockEffectGenerator.AddTexture(LoadHollowTextureFrom("Asset/World/Texture/BuiltinBlock/Leaf.png").raw_data());
        int textureIndices[] = { textureIndex, textureIndex, textureIndex, textureIndex, textureIndex, textureIndex };

        auto leafDesc = std::make_shared<DiffuseHollowBoxDescription>(
            "leaf", diffuseHollowEffect, textureIndices, BLOCK_BRIGHTNESS_MIN, BlockBrightness{ 1, 1, 1, 1 });
        descMgr.RegisterBlockDescription(leafDesc);
        info_[int(BuiltinBlockType::Leaf)].desc = leafDesc;
	}

    if(!diffuseHollowBlockEffectGenerator.IsEmpty())
    {
        diffuseHollowBlockEffectGenerator.InitializeEffect(*diffuseHollowEffect);
        effectMgr.RegisterBlockEffect(diffuseHollowEffect);
    }

    if(!diffuseSolidEffectGenerator.IsEmpty())
    {
        diffuseSolidEffectGenerator.InitializeEffect(*diffuseSolidEffect);
        effectMgr.RegisterBlockEffect(diffuseSolidEffect);
    }
}

VRPG_WORLD_END
