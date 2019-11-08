#include <VRPG/World/Land/FlatLandGenerator.h>
#include <VRPG/World/Game.h>

VRPG_WORLD_BEGIN

Game::Game(Base::Window *window)
    : window_(window), keyboard_(window->GetKeyboard()), mouse_(window->GetMouse()),
      worldTickInterval_(100000), exitMainloop_(false)
{
    assert(window);
}

void Game::Run()
{
    Initialize();
    AGZ_SCOPE_GUARD({
        Destroy();
    });

    int cameraBlockX = int(camera_->GetPosition().x), cameraBlockZ = int(camera_->GetPosition().z);
    chunkManager_->SetCentreChunk(GlobalBlockToChunk(cameraBlockX, cameraBlockZ));

    ScalarHistory frameTimeHistory(4);

    exitMainloop_ = false;

    mouse_->ShowCursor(false);
    mouse_->SetCursorLock(true, window_->GetClientSizeX() / 2, window_->GetClientSizeY() / 2);
    mouse_->UpdatePosition();

    spdlog::info("start mainloop");

    using Clock = std::chrono::high_resolution_clock;
    Clock::time_point lastWorldTickEnd = Clock::now();
    fpsCounter_.restart();

    while(!exitMainloop_)
    {
        float deltaT = fpsCounter_.elasped_microseconds() / 1000.0f;
        frameTimeHistory.Update(deltaT);
        deltaT = frameTimeHistory.MeanValue();

        window_->DoEvents();
        window_->WaitForFocus();
        window_->ImGuiNewFrame();

        if(keyboard_->IsKeyPressed(Base::KEY_ESCAPE) || window_->GetCloseFlag())
            exitMainloop_ = true;

        PlayerTick(deltaT);

        {
            us worldTickDelta = std::chrono::duration_cast<us>(Clock::now() - lastWorldTickEnd);
            if(worldTickDelta > 10 * worldTickInterval_)
                worldTickDelta = worldTickInterval_;
            while(worldTickDelta >= worldTickInterval_)
            {
                WorldTick();
                worldTickDelta -= worldTickInterval_;
                lastWorldTickEnd = Clock::now();
            }
        }

        Render();
    }

    spdlog::info("exit mainloop");
}

void Game::Initialize()
{
    spdlog::info("initialize immediate2D && crosshair painter");

    imm2D_ = std::make_unique<Immediate2D>();
    crosshairPainter_ = std::make_unique<Crosshair>();

    spdlog::info("initialize camera");

    camera_ = std::make_unique<DefaultCamera>();
    camera_->SetMoveSpeed(7);
    camera_->SetViewSpeed(0.003f);
    camera_->SetWOverH(window_->GetClientAspectRatio());
    camera_->SetPosition({ 0, 30, 0 });
    camera_->SetClipDistance(0.1f, 1000.0f);

    spdlog::info("initialize chunk renderer");

    chunkRenderer_ = std::make_unique<ChunkRenderer>();

    spdlog::info("initialize chunk manager");

    ChunkManagerParams chunkMgrParams;
    chunkMgrParams.unloadDistance        = 12;
    chunkMgrParams.loadDistance          = 10;
    chunkMgrParams.renderDistance        = 9;
    chunkMgrParams.backgroundPoolSize    = 20;
    chunkMgrParams.backgroundThreadCount = 1;
    chunkManager_ = std::make_unique<ChunkManager>(chunkMgrParams, std::make_unique<FlatLandGenerator>(20));

    worldTickInterval_ = us(50 * 1000);
}

void Game::PlayerTick(float deltaT)
{
    DefaultCamera::Input cameraInput;
    cameraInput.front = keyboard_->IsKeyPressed('W');
    cameraInput.back  = keyboard_->IsKeyPressed('S');
    cameraInput.left  = keyboard_->IsKeyPressed('A');
    cameraInput.right = keyboard_->IsKeyPressed('D');
    cameraInput.up    = keyboard_->IsKeyPressed(Base::KEY_SPACE);
    cameraInput.down  = keyboard_->IsKeyPressed(Base::KEY_LSHIFT);

    cameraInput.relativeCursorX = float(mouse_->GetRelativeCursorPositionX());
    cameraInput.relativeCursorY = float(mouse_->GetRelativeCursorPositionY());

    camera_->Update(cameraInput, deltaT);

    static bool isLBDown    = false;
    static bool isLBPressed = false;

    isLBDown = !isLBPressed && mouse_->IsMouseButtonPressed(Base::MouseButton::Left);
    isLBPressed = mouse_->IsMouseButtonPressed(Base::MouseButton::Left);

    Vec3i pickedBlockPosition;
    if(chunkManager_->FindClosestIntersectedBlock(camera_->GetPosition(), camera_->GetDirection(), 8.0f, &pickedBlockPosition))
    {
        if(isLBDown)
            chunkManager_->SetBlockID(pickedBlockPosition, BLOCK_ID_VOID, {});
    }
}

void Game::WorldTick()
{
    int cameraBlockX = int(camera_->GetPosition().x), cameraBlockZ = int(camera_->GetPosition().z);
    chunkManager_->SetCentreChunk(GlobalBlockToChunk(cameraBlockX, cameraBlockZ));

    bool needToGenerateRenderer = false;
    needToGenerateRenderer |= chunkManager_->UpdateChunkData();
    needToGenerateRenderer |= chunkManager_->UpdateChunkModels();

    if(needToGenerateRenderer)
    {
        chunkRenderer_->Clear();
        chunkManager_->FillRenderer(*chunkRenderer_);
        chunkRenderer_->Done();
    }
}

void Game::Render()
{
    static const Vec4 backgroundColor = Vec4(0.7f, 1, 1, 0).map([](float x) { return std::pow(x, 1 / 2.2f); });
    window_->ClearDefaultRenderTarget(&backgroundColor[0]);
    window_->ClearDefaultDepthStencil();

    chunkRenderer_->Render({ camera_.get(), Vec3(1) });

    crosshairPainter_->Draw(*imm2D_);

    window_->ImGuiRender();
    window_->SwapBuffers();
}

void Game::Destroy()
{
    spdlog::info("destroy immediate2D && crosshairPainter");
    imm2D_.reset();
    crosshairPainter_.reset();

    spdlog::info("destroy camera");
    camera_.reset();

    spdlog::info("destroy chunk renderer");
    chunkRenderer_.reset();

    spdlog::info("destroy chunk manager");
    chunkManager_.reset();
}

VRPG_WORLD_END
