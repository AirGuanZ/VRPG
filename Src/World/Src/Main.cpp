#include <iostream>

#include <VRPG/Base/Base.h>

#include <VRPG/World/Chunk/ChunkManager.h>
#include <VRPG/World/Land/FlatLandGenerator.h>

#include <VRPG/World/Block/BasicEffect/DefaultBlockEffect.h>
#include <VRPG/World/Block/BasicDescription/DefaultBlockDescription.h>

void Run()
{
    using namespace VRPG::Base;
    using namespace VRPG::World;

    Window window;
    WindowDesc desc;
    window.Initialize(desc);

    KeyboardEventManager *keyboard = window.GetKeyboard();

    auto defaultBlockEffect = std::make_shared<DefaultBlockEffect>();
    BlockEffectManager::GetInstance().RegisterBlockEffect(defaultBlockEffect);
    BlockDescriptionManager::GetInstance().RegisterBlockDescription(std::make_shared<DefaultBlockDescription>(defaultBlockEffect.get()));

    ChunkManagerParams params = {};
    params.loadDistance          = 10;
    params.backgroundPoolSize    = 100;
    params.backgroundThreadCount = 1;
    params.renderDistance        = 9;
    params.unloadDistance        = 12;
    ChunkManager chunkMgr(params, std::make_unique<FlatLandGenerator>(20));
    chunkMgr.SetCentreChunk(1, 0);

    spdlog::info("start mainloop");

    while(!window.GetCloseFlag())
    {
        window.DoEvents();

        chunkMgr.UpdateChunkData();

        if(keyboard->IsKeyPressed(KEY_ESCAPE))
            window.SetCloseFlag(true);

        window.ClearDefaultRenderTarget();
        window.SwapBuffers();
    }

    spdlog::info("stop mainloop");

    BlockEffectManager::GetInstance().Clear();
    BlockDescriptionManager::GetInstance().Clear();
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
