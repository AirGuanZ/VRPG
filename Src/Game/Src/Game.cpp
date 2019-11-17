#include <ImGui/imgui.h>

#include <VRPG/Game/Block/BuiltinBlock/BuiltinBlock.h>
#include <VRPG/Game/Config/GlobalConfig.h>
#include <VRPG/Game/Land/FlatLandGenerator.h>
#include <VRPG/Game/World/BlockUpdater/LiquidUpdater.h>
#include <VRPG/Game/Game.h>

VRPG_GAME_BEGIN

Game::Game(Base::Window *window)
    : window_(window), keyboard_(window->GetKeyboard()), mouse_(window->GetMouse())
{
    assert(window);
}

void Game::Run()
{
    Initialize();
    AGZ_SCOPE_GUARD({
        Destroy();
    });

    spdlog::info("start mainloop");

    agz::time::fps_counter_t fpsCounter;
    auto lastWorldTick = StdClock::now();

    bool exitMainloop = false;
    while(!exitMainloop)
    {
        float deltaT = fpsCounter.elasped_microseconds() / 1000.0f;

        window_->DoEvents();
        window_->WaitForFocus();
        window_->ImGuiNewFrame();

        if(keyboard_->IsKeyPressed(Base::KEY_ESCAPE) || window_->GetCloseFlag())
            exitMainloop = true;

        PlayerTick(deltaT);

        {
            us worldTickDelta = std::chrono::duration_cast<us>(StdClock::now() - lastWorldTick);
            if(worldTickDelta > 10 * WORLD_TICK_INTERVAL)
                worldTickDelta = WORLD_TICK_INTERVAL;
            while(worldTickDelta >= WORLD_TICK_INTERVAL)
            {
                WorldTick();
                worldTickDelta -= WORLD_TICK_INTERVAL;
                lastWorldTick = StdClock::now();
            }
        }

        ChunkTick();

        Render(fpsCounter.fps());

        fpsCounter.frame_end();
    }

    spdlog::info("exit mainloop");
}

void Game::Initialize()
{
    spdlog::info("initialize immediate2D");
    imm2D_ = std::make_unique<Immediate2D>();

    spdlog::info("initialize crosshair painter");
    crosshairPainter_ = std::make_unique<Crosshair>();

    spdlog::info("initialize camera");

    camera_ = std::make_unique<DefaultCamera>();
    camera_->SetMoveSpeed(7);
    camera_->SetViewSpeed(0.002f);
    camera_->SetWOverH(window_->GetClientAspectRatio());
    camera_->SetPosition({ 0, 30, 0 });
    camera_->SetFOVy(70);
    camera_->SetClipDistance(0.1f, 1000.0f);

    spdlog::info("initialize shadow map");
    CSM_ = std::make_unique<CascadeShadowMapping>();

    spdlog::info("initialize chunk renderer");
    chunkRenderer_ = std::make_unique<ChunkRenderer>();

    spdlog::info("initialize chunk manager");

    ChunkManagerParams chunkMgrParams;
    chunkMgrParams.unloadDistance        = GLOBAL_CONFIG.CHUNK_MANAGER.unloadDistance;
    chunkMgrParams.loadDistance          = GLOBAL_CONFIG.CHUNK_MANAGER.loadDistance;
    chunkMgrParams.renderDistance        = GLOBAL_CONFIG.CHUNK_MANAGER.renderDistance;
    chunkMgrParams.backgroundPoolSize    = GLOBAL_CONFIG.CHUNK_MANAGER.backgroundPoolSize;
    chunkMgrParams.backgroundThreadCount = GLOBAL_CONFIG.CHUNK_MANAGER.backgroundThreadCount;
    chunkManager_ = std::make_unique<ChunkManager>(chunkMgrParams, std::make_unique<FlatLandGenerator>(20));

    spdlog::info("initialize block updater");
    blockUpdaterManager_ = std::make_unique<BlockUpdaterManager>(chunkManager_.get());

    spdlog::info("initialize chosen block wireframe renderer");
    chosenBlockWireframeRenderer_ = std::make_unique<ChosenWireframeRenderer>();

    HideCursor();
    UpdateCentreChunk();
}

void Game::Destroy()
{
    spdlog::info("destroy block updater manager");
    blockUpdaterManager_.reset();

    spdlog::info("destroy immediate2D && crosshairPainter");
    imm2D_.reset();
    crosshairPainter_.reset();

    spdlog::info("destroy camera");
    camera_.reset();

    spdlog::info("destroy shadow map");
    CSM_.reset();

    spdlog::info("destroy chunk renderer");
    chunkRenderer_.reset();

    spdlog::info("destroy chunk manager");
    chunkManager_.reset();
}

void Game::PlayerTick(float deltaT)
{
    UpdateCamera(deltaT);

    Vec3i pickedBlockPosition; Direction pickedFace = PositiveX;
    if(chunkManager_->FindClosestIntersectedBlock(
        camera_->GetPosition(), camera_->GetDirection(), 8.0f, &pickedBlockPosition, &pickedFace))
    {
        chosenBlockPosition_ = pickedBlockPosition;

        if(mouse_->IsMouseButtonDown(Base::MouseButton::Left))
        {
            chunkManager_->SetBlockID(pickedBlockPosition, BLOCK_ID_VOID, {});

            LiquidUpdater::AddUpdaterForNeighborhood(
                pickedBlockPosition, *blockUpdaterManager_, *chunkManager_, StdClock::now());
        }
        else if(mouse_->IsMouseButtonDown(Base::MouseButton::Right))
        {
            Vec3i newBlockPosition = pickedBlockPosition;
            switch(pickedFace)
            {
            case PositiveX: newBlockPosition.x += 1; break;
            case NegativeX: newBlockPosition.x -= 1; break;
            case PositiveY: newBlockPosition.y += 1; break;
            case NegativeY: newBlockPosition.y -= 1; break;
            case PositiveZ: newBlockPosition.z += 1; break;
            case NegativeZ: newBlockPosition.z -= 1; break;
            }

            if(chunkManager_->GetBlockDesc(newBlockPosition)->IsReplacableByCrosshairRay())
            {
                /*auto waterDesc = BuiltinBlockTypeManager::GetInstance().GetDesc(BuiltinBlockType::Water);
                auto waterID   = waterDesc->GetBlockID();
                auto extraData = MakeLiquidExtraData(waterDesc->GetLiquidDescription()->sourceLevel);
                chunkManager_->SetBlockID(
                    newBlockPosition, waterID, {}, std::move(extraData));
                LiquidUpdater::AddUpdaterForNeighborhood(
                    newBlockPosition, *blockUpdaterManager_, *chunkManager_, StdClock::now());*/

                auto stoneID = BuiltinBlockTypeManager::GetInstance().GetID(BuiltinBlockType::Stone);
                chunkManager_->SetBlockID(newBlockPosition, stoneID, {});
            }
        }
    }
    else
        chosenBlockPosition_ = std::nullopt;
}

void Game::WorldTick()
{
    blockUpdaterManager_->Execute(20, StdClock::now());
}

void Game::ChunkTick()
{
    int cameraBlockX = int(camera_->GetPosition().x), cameraBlockZ = int(camera_->GetPosition().z);
    chunkManager_->SetCentreChunk(GlobalBlockToChunk(cameraBlockX, cameraBlockZ));

    bool needToGenerateRenderer = false;
    needToGenerateRenderer |= chunkManager_->UpdateChunkData();
    chunkManager_->UpdateLight();
    needToGenerateRenderer |= chunkManager_->UpdateChunkModels();

    if(needToGenerateRenderer)
    {
        chunkRenderer_->Clear();
        chunkManager_->FillRenderer(*chunkRenderer_);
        chunkRenderer_->Done();
    }
}

void Game::Render(int fps)
{
    constexpr auto WIN_FLAG =
        ImGuiWindowFlags_NoMove             |
        ImGuiWindowFlags_NoTitleBar         |
        ImGuiWindowFlags_NoResize           |
        ImGuiWindowFlags_AlwaysAutoResize   |
        ImGuiWindowFlags_NoSavedSettings    |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav;
    ImGui::SetNextWindowBgAlpha(0.3f);
    ImGui::SetNextWindowPos({ 20, 20 });
    if(ImGui::Begin("debug overlay", nullptr, WIN_FLAG))
    {
        ImGui::Text("fps: %i", fps);

        Vec3 position = camera_->GetPosition();
        ImGui::Text("position: (%f, %f, %f)", position.x, position.y, position.z);

        Vec3 direction = camera_->GetDirection();
        ImGui::Text("direction: (%f, %f, %f)", direction.x, direction.y, direction.z);
    }
    ImGui::End();

    CSM_->RenderShadow(*camera_, *chunkRenderer_);

    static const Vec4 backgroundColor = Vec4(0.7f, 1, 1, 0).map(
        [](float x) { return std::pow(x, 1 / 2.2f); });
    window_->ClearDefaultRenderTarget(&backgroundColor[0]);
    window_->ClearDefaultDepthStencil();

    {
        BlockForwardRenderParams params;
        params.camera = camera_.get();
        params.skyLight = Vec3(1);
        CSM_->FillForwardParams(params);
        chunkRenderer_->RenderForward(params);
    }

    if(GLOBAL_CONFIG.MISC.enableChoseBlockWireframe && chosenBlockPosition_)
        chosenBlockWireframeRenderer_->DrawBlockWireframeAt(*camera_, *chosenBlockPosition_);

    crosshairPainter_->Draw(*imm2D_);

    window_->ImGuiRender();
    window_->SwapBuffers();
}

void Game::HideCursor()
{
    int lockX = window_->GetClientSizeX() / 2;
    int lockY = window_->GetClientSizeY() / 2;
    mouse_->ShowCursor(false);
    mouse_->SetCursorLock(true, lockX, lockY);
    mouse_->Update();
}

void Game::UpdateCamera(float deltaT)
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

    camera_->SetWOverH(window_->GetClientAspectRatio());
    camera_->Update(cameraInput, deltaT);
}

void Game::UpdateCentreChunk()
{
    int cameraBlockX = int(camera_->GetPosition().x), cameraBlockZ = int(camera_->GetPosition().z);
    chunkManager_->SetCentreChunk(GlobalBlockToChunk(cameraBlockX, cameraBlockZ));
}

VRPG_GAME_END
