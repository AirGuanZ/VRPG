#include <iostream>

#include <agz/utility/time.h>

#include <VRPG/Base/Base.h>

#include <VRPG/Game/Block/BuiltinBlock/BuiltinBlock.h>
#include <VRPG/Game/Block/BlockDescription.h>
#include <VRPG/Game/Block/BlockEffect.h>
#include <VRPG/Game/Game.h>

void Run()
{
    using namespace VRPG::Base;
    using namespace VRPG::World;

    Window window;
    WindowDesc desc;
    desc.clientWidth  = 900;
    desc.clientHeight = 600;
    desc.sampleCount  = 4;
    window.Initialize(desc);

	spdlog::info("initialize builtin block manager");
	BuiltinBlockTypeManager::GetInstance().RegisterBuiltinBlockTypes();
	AGZ_SCOPE_GUARD({
		spdlog::info("destroy builtin block manager");
		BuiltinBlockTypeManager::GetInstance().Clear();
		BlockEffectManager::GetInstance().Clear();
		BlockDescriptionManager::GetInstance().Clear();
	});

    Game game(&window);
    game.Run();
}

int main()
{
    try
    {
        Run();
    }
    catch(const std::exception &err)
    {
        std::cout << err.what() << std::endl;
        return 1;
    }
}
