#include <iostream>

#include <VRPG/Base/Base.h>

#include <VRPG/Game/Block/BuiltinBlock/BuiltinBlock.h>
#include <VRPG/Game/Block/BlockDescription.h>
#include <VRPG/Game/Block/BlockEffect.h>
#include <VRPG/Game/Config/GlobalConfig.h>
#include <VRPG/Game/Game.h>

int main()
{
    try
    {
        using namespace VRPG::Base;
        using namespace VRPG::World;

        GLOBAL_CONFIG.LoadFromFile("config.cfg");
        GLOBAL_CONFIG.ASSET_PATH.LoadFromFile("asset.cfg");

        Window window;
        WindowDesc desc;
        desc.clientWidth   = GLOBAL_CONFIG.WINDOW.width;
        desc.clientHeight  = GLOBAL_CONFIG.WINDOW.height;
        desc.fullscreen    = GLOBAL_CONFIG.WINDOW.fullscreen;
        desc.vsync         = GLOBAL_CONFIG.WINDOW.vsync;
        desc.sampleCount   = GLOBAL_CONFIG.WINDOW.sampleCount;
        desc.sampleQuality = GLOBAL_CONFIG.WINDOW.sampleQuality;
        window.Initialize(desc);

        spdlog::info("initialize builtin block manager");
        BuiltinBlockTypeManager::GetInstance().RegisterBuiltinBlockTypes();
        AGZ_SCOPE_GUARD({
            spdlog::info("destroy builtin block manager");
            BuiltinBlockTypeManager::GetInstance().Clear();
            BlockEffectManager     ::GetInstance().Clear();
            BlockDescriptionManager::GetInstance().Clear();
        });

        Game game(&window);
        game.Run();
    }
    catch(const libconfig::SettingException &err)
    {
        std::cout << err.what() << ": " << err.getPath() << std::endl;
        return 1;
    }
    catch(const std::exception &err)
    {
        std::cout << err.what() << std::endl;
        return 1;
    }
}
