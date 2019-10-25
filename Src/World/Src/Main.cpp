#include <iostream>

#include <VRPG/Base/Base.h>

#include <VRPG/World/Chunk/ChunkManager.h>

class Land : public VRPG::World::LandGenerator
{
public:

    void Generate(const VRPG::World::ChunkPosition& position, VRPG::World::ChunkBlockData* blockData) override
    {
        
    }
};

void Run()
{
    using namespace VRPG::Base;
    using namespace VRPG::World;

    Window window;
    WindowDesc desc;
    window.Initialize(desc);

    KeyboardEventManager *keyboard = window.GetKeyboard();

    ChunkManagerParams params;
    params.loadDistance = 10;
    params.backgroundPoolSize = 100;
    params.backgroundThreadCount = 2;
    params.renderDistance = 9;
    params.unloadDistance = 12;
    ChunkManager chunkMgr(params, std::make_unique<Land>());
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
