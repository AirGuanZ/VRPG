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

        GlobalGraphicsConfig::GetInstance().LoadFromFile("config.cfg");

        Window window;
        WindowDesc desc;
        desc.clientWidth  = 1024;
        desc.clientHeight = 768;
        desc.sampleCount  = 4;
        //desc.fullscreen = true;
        //desc.vsync = false;
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
    catch(const std::exception &err)
    {
        std::cout << err.what() << std::endl;
        return 1;
    }
}
