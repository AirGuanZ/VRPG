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
    lbState_ = MouseButtonStateTracker(Base::MouseButton::Left);
    rbState_ = MouseButtonStateTracker(Base::MouseButton::Right);
}

void Game::Run()
{
    Initialize();
    AGZ_SCOPE_GUARD({
        Destroy();
    });

    int cameraBlockX = int(camera_->GetPosition().x), cameraBlockZ = int(camera_->GetPosition().z);
    chunkManager_->SetCentreChunk(GlobalBlockToChunk(cameraBlockX, cameraBlockZ));

    mouse_->ShowCursor(false);
    mouse_->SetCursorLock(true, window_->GetClientSizeX() / 2, window_->GetClientSizeY() / 2);
    mouse_->UpdatePosition();

    spdlog::info("start mainloop");

    using Clock = std::chrono::high_resolution_clock;
    Clock::time_point lastWorldTickEnd = Clock::now();
    
    agz::time::fps_counter_t fpsCounter;

    bool exitMainloop = false;
    while(!exitMainloop)
    {
        float deltaT = fpsCounter.elasped_microseconds() / 1000.0f;

        window_->DoEvents();
        window_->WaitForFocus();
        window_->ImGuiNewFrame();

        lbState_.Update(mouse_);
        rbState_.Update(mouse_);

        if(keyboard_->IsKeyPressed(Base::KEY_ESCAPE) || window_->GetCloseFlag())
            exitMainloop = true;

        PlayerTick(deltaT);

        {
            us worldTickDelta = std::chrono::duration_cast<us>(Clock::now() - lastWorldTickEnd);
            if(worldTickDelta > 10 * WORLD_TICK_INTERVAL)
                worldTickDelta = WORLD_TICK_INTERVAL;
            while(worldTickDelta >= WORLD_TICK_INTERVAL)
            {
                WorldTick();
                worldTickDelta -= WORLD_TICK_INTERVAL;
                lastWorldTickEnd = Clock::now();
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
    spdlog::info("initialize immediate2D && crosshair painter");

    imm2D_ = std::make_unique<Immediate2D>();
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

    shadowMap_ = std::make_unique<Base::ShadowMap>(4096, 4096);

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

    blockUpdaterManager_ = std::make_unique<BlockUpdaterManager>(chunkManager_.get());
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

    camera_->SetWOverH(window_->GetClientAspectRatio());
    camera_->Update(cameraInput, deltaT);

    Vec3i pickedBlockPosition; Direction pickedFace = PositiveX;
    if(chunkManager_->FindClosestIntersectedBlock(camera_->GetPosition(), camera_->GetDirection(), 8.0f, &pickedBlockPosition, &pickedFace))
    {
        if(lbState_.IsDown())
        {
            chunkManager_->SetBlockID(pickedBlockPosition, BLOCK_ID_VOID, {});

            LiquidUpdater::AddUpdaterForNeighborhood(pickedBlockPosition, *blockUpdaterManager_, *chunkManager_, StdClock::now());
        }
        else if(rbState_.IsDown())
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

            if(chunkManager_->GetBlockDesc(newBlockPosition)->IsReplacable())
            {
                auto waterDesc = BuiltinBlockTypeManager::GetInstance().GetDesc(BuiltinBlockType::Water).desc;
                BlockID waterID = waterDesc->GetBlockID();
                chunkManager_->SetBlockID(newBlockPosition, waterID, {}, MakeLiquidExtraData(waterDesc->GetLiquidDescription()->sourceLevel));
                LiquidUpdater::AddUpdaterForNeighborhood(newBlockPosition, *blockUpdaterManager_, *chunkManager_, StdClock::now());

                /*auto stoneID = BuiltinBlockTypeManager::GetInstance().GetDesc(BuiltinBlockType::Stone).desc->GetBlockID();
                chunkManager_->SetBlockID(newBlockPosition, stoneID, {});*/
            }
        }
    }
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
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings |
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

    shadowMap_->Begin();

    Mat4 shadowMapVP = ConstructShadowMapVP();
    chunkRenderer_->RenderShadow({ shadowMapVP });

    shadowMap_->End();

    static const Vec4 backgroundColor = Vec4(0.7f, 1, 1, 0).map([](float x) { return std::pow(x, 1 / 2.2f); });
    window_->ClearDefaultRenderTarget(&backgroundColor[0]);
    window_->ClearDefaultDepthStencil();

    chunkRenderer_->RenderForward({
        camera_.get(),
        Vec3(1),
        Vec3(5, 6, 7).normalize(),
        0.2f,
        shadowMapVP,
        shadowMap_->GetSRV(),
        1.0f / 4096
    });

    crosshairPainter_->Draw(*imm2D_);

    window_->ImGuiRender();
    window_->SwapBuffers();
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
    shadowMap_.reset();

    spdlog::info("destroy chunk renderer");
    chunkRenderer_.reset();

    spdlog::info("destroy chunk manager");
    chunkManager_.reset();
}

Mat4 Game::ConstructShadowMapVP() const
{
    float distance = GLOBAL_CONFIG.SHADOW_MAP.shadowMapDistance;
    float radius   = GLOBAL_CONFIG.SHADOW_MAP.shadowMapRadius;
    float nearP    = GLOBAL_CONFIG.SHADOW_MAP.shadowMapNear;
    float farP     = GLOBAL_CONFIG.SHADOW_MAP.shadowMapFar;

    Vec3 cameraPosition = camera_->GetPosition();
    Mat4 view = Trans4::look_at(cameraPosition + distance * Vec3(5, 6, 7).normalize(), cameraPosition, Vec3(0, 1, 0));
    Mat4 proj = Trans4::orthographic(-radius, +radius, +radius, -radius, nearP, farP);
    return view * proj;
}

VRPG_GAME_END
