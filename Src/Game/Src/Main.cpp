#include <iostream>

#include <VRPG/Base/Base.h>

#include <VRPG/Game/Config/GlobalConfig.h>
#include <VRPG/Game/Game.h>
#include <VRPG/Game/World/Block/BlockDescription.h>
#include <VRPG/Game/World/Block/BlockEffect.h>
#include <VRPG/Game/World/Block/BuiltinBlock.h>

int main()
{
    /*using namespace VRPG::Base;

    auto q1 = Quaternion::construct_quaternion({ 1, 2, 3 }, 56);
    auto q2 = Quaternion::construct_quaternion({ 0, 0, 1 }, 0.1f);

    auto q3 = Quaternion::construct_quaternion(Vec3(1, 2, 3).normalize(), 0.6f);
    auto q4 = Quaternion::construct_quaternion(Vec3(1, 2, 3).normalize(), 0.9f);
    auto q5 = slerp(q3, q4, 0.3f);
    auto q6 = Quaternion::construct_quaternion(Vec3(1, 2, 3).normalize(), 0.69f);

    std::cout << q3.a << " " << q3.b << std::endl;
    std::cout << q4.a << " " << q4.b << std::endl;
    std::cout << q5.a << " " << q5.b << std::endl;
    std::cout << q6.a << " " << q6.b << std::endl;

    std::cout << q1.apply_to_vector({ 3, 2, 1 }) << std::endl;
    std::cout << q6.apply_to_vector({ 3, 2, 1 }) << std::endl;

    std::cout << (Vec4(3, 2, 1, 0) * Trans4::from_quaternion(q1)).xyz() << std::endl;
    std::cout << (Vec4(3, 2, 1, 0) * Trans4::from_quaternion(q6)).xyz() << std::endl;

    std::cout << (Vec4(3, 2, 1, 0) * Trans4::rotate({ 1, 2, 3 }, 56)).xyz() << std::endl;
    std::cout << (Vec4(3, 2, 1, 0) * Trans4::rotate({ 1, 2, 3 }, 0.69f)).xyz() << std::endl;

    return 0;*/

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
            BlockDescManager       ::GetInstance().Clear();
        });

        Game game(&window);
        game.Run();
    }
    catch(const libconfig::SettingException &err)
    {
        std::cout << err.what() << ": " << err.getPath() << std::endl;
        return 1;
    }
    catch(const libconfig::ParseException &err)
    {
        std::cout << err.what() << ": " << err.getLine() << ", " << err.getError() << std::endl;
    }
    catch(const std::exception &err)
    {
        std::cout << err.what() << std::endl;
        return 1;
    }
}
