#include <ImGui/imgui.h>

#include <VRPG/Game/Land/FlatLandGenerator.h>
#include <VRPG/Game/Game.h>

VRPG_GAME_BEGIN

Game::Game(Base::Window *window)
    : window_(window), keyboard_(window->GetKeyboard()), mouse_(window->GetMouse()),
      worldTickInterval_(100000), exitMainloop_(false)
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

        window_->DoEvents();
        window_->WaitForFocus();
        window_->ImGuiNewFrame();

        lbState_.Update(mouse_);
        rbState_.Update(mouse_);

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

        ChunkTick();

        Render();

        fpsCounter_.frame_end();
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

    camera_->SetWOverH(window_->GetClientAspectRatio());
    camera_->Update(cameraInput, deltaT);

    Vec3i pickedBlockPosition; Direction pickedFace = PositiveX;
    if(chunkManager_->FindClosestIntersectedBlock(camera_->GetPosition(), camera_->GetDirection(), 8.0f, &pickedBlockPosition, &pickedFace))
    {
        if(lbState_.IsDown())
            chunkManager_->SetBlockID(pickedBlockPosition, BLOCK_ID_VOID, {});
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
            chunkManager_->SetBlockID(newBlockPosition, 2, {});
        }
    }
}

void Game::WorldTick()
{

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

void Game::Render()
{
    constexpr auto WIN_FLAG =
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav;
    ImGui::SetNextWindowBgAlpha(0.2f);
    ImGui::SetNextWindowPos({ 20, 20 });
    if(ImGui::Begin("debug overlay", nullptr, WIN_FLAG))
    {
        ImGui::Text("fps: %i", fpsCounter_.fps());

        Vec3 position = camera_->GetPosition();
        ImGui::Text("position: (%f, %f, %f)", position.x, position.y, position.z);
    }
    ImGui::End();

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

VRPG_GAME_END
