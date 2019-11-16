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
    StdClock::time_point lastWorldTick = StdClock::now();

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

    nearShadowMap_   = std::make_unique<Base::ShadowMap>(
        GLOBAL_CONFIG.SHADOW_MAP.resolution[0], GLOBAL_CONFIG.SHADOW_MAP.resolution[0]);
    middleShadowMap_ = std::make_unique<Base::ShadowMap>(
        GLOBAL_CONFIG.SHADOW_MAP.resolution[1], GLOBAL_CONFIG.SHADOW_MAP.resolution[1]);
    farShadowMap_    = std::make_unique<Base::ShadowMap>(
        GLOBAL_CONFIG.SHADOW_MAP.resolution[2], GLOBAL_CONFIG.SHADOW_MAP.resolution[2]);

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

    HideCursor();
    UpdateCentreChunk();
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
    if(chunkManager_->FindClosestIntersectedBlock(
        camera_->GetPosition(), camera_->GetDirection(), 8.0f, &pickedBlockPosition, &pickedFace))
    {
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

    Mat4 shadowVP[3]; float homZLimit[3];
    ConstructShadowMapVP(shadowVP, homZLimit);

    {
        nearShadowMap_->Begin();
        chunkRenderer_->RenderShadow({ shadowVP[0] });
        nearShadowMap_->End();

        middleShadowMap_->Begin();
        chunkRenderer_->RenderShadow({ shadowVP[1] });
        middleShadowMap_->End();

        farShadowMap_->Begin();
        chunkRenderer_->RenderShadow({ shadowVP[2] });
        farShadowMap_->End();
    }

    static const Vec4 backgroundColor = Vec4(0.7f, 1, 1, 0).map(
        [](float x) { return std::pow(x, 1 / 2.2f); });
    window_->ClearDefaultRenderTarget(&backgroundColor[0]);
    window_->ClearDefaultDepthStencil();

    {
        BlockForwardRenderParams params;
        params.camera = camera_.get();
        params.skyLight = Vec3(1);
        params.shadowScale = 0.2f;
        params.sunlightDirection = Vec3(5, 6, 7).normalize();

        params.cascadeShadowMaps[0].shadowMapSRV = nearShadowMap_->GetSRV();
        params.cascadeShadowMaps[1].shadowMapSRV = middleShadowMap_->GetSRV();
        params.cascadeShadowMaps[2].shadowMapSRV = farShadowMap_->GetSRV();

        params.cascadeShadowMaps[0].PCFStep = 1.0f / GLOBAL_CONFIG.SHADOW_MAP.resolution[0];
        params.cascadeShadowMaps[1].PCFStep = 1.0f / GLOBAL_CONFIG.SHADOW_MAP.resolution[1];
        params.cascadeShadowMaps[2].PCFStep = 1.0f / GLOBAL_CONFIG.SHADOW_MAP.resolution[2];

        params.cascadeShadowMaps[0].shadowViewProj = shadowVP[0];
        params.cascadeShadowMaps[1].shadowViewProj = shadowVP[1];
        params.cascadeShadowMaps[2].shadowViewProj = shadowVP[2];

        params.cascadeShadowMaps[0].homZLimit = homZLimit[0];
        params.cascadeShadowMaps[1].homZLimit = homZLimit[1];
        params.cascadeShadowMaps[2].homZLimit = homZLimit[2];

        chunkRenderer_->RenderForward(params);
    }

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
    nearShadowMap_.reset();
    middleShadowMap_.reset();
    farShadowMap_.reset();

    spdlog::info("destroy chunk renderer");
    chunkRenderer_.reset();

    spdlog::info("destroy chunk manager");
    chunkManager_.reset();
}

void Game::HideCursor()
{
    int lockX = window_->GetClientSizeX() / 2;
    int lockY = window_->GetClientSizeY() / 2;
    mouse_->ShowCursor(false);
    mouse_->SetCursorLock(true, lockX, lockY);
    mouse_->Update();
}

void Game::UpdateCentreChunk()
{
    int cameraBlockX = int(camera_->GetPosition().x), cameraBlockZ = int(camera_->GetPosition().z);
    chunkManager_->SetCentreChunk(GlobalBlockToChunk(cameraBlockX, cameraBlockZ));
}

void Game::ConstructShadowMapVP(Mat4 VP[3], float homZLimit[3]) const
{
    Vec3 cameraPosition = camera_->GetPosition();
    float FOVy          = camera_->GetFOVy();
    float wOverH        = camera_->GetWOverH();

    Mat4 shadowView = Trans4::look_at(
        cameraPosition + 500.0f * Vec3(5, 6, 7).normalize(),
        cameraPosition, Vec3(0, 1, 0));
    Mat4 viewToShadow = camera_->GetViewMatrix().inv() * shadowView;

    auto constructSingleShadowMapVP = [&](float nearD, float farD)
    {
        float nearYOri = nearD * std::tan(FOVy / 2);
        float nearXOri = wOverH * nearYOri;
        Vec3 nearPoints[4] =
        {
            { +nearXOri, +nearYOri, nearD },
            { +nearXOri, -nearYOri, nearD },
            { -nearXOri, +nearYOri, nearD },
            { -nearXOri, -nearYOri, nearD }
        };

        float farYOri = farD * std::tan(FOVy / 2);
        float farXOri = wOverH * farYOri;
        Vec3 farPoints[4] =
        {
            { +farXOri, +farYOri, farD },
            { +farXOri, -farYOri, farD },
            { -farXOri, +farYOri, farD },
            { -farXOri, -farYOri, farD }
        };

        for(int i = 0; i < 4; ++i)
        {
            Vec4 vn = Vec4(nearPoints[i].x, nearPoints[i].y, nearPoints[i].z, 1);
            nearPoints[i] = (vn * viewToShadow).xyz();

            Vec4 vf = Vec4(farPoints[i].x, farPoints[i].y, farPoints[i].z, 1);
            farPoints[i] = (vf * viewToShadow).xyz();
        }

        float minX = (std::numeric_limits<float>::max)();
        float minY = (std::numeric_limits<float>::max)();
        float minZ = (std::numeric_limits<float>::max)();
        float maxX = (std::numeric_limits<float>::lowest)();
        float maxY = (std::numeric_limits<float>::lowest)();
        float maxZ = (std::numeric_limits<float>::lowest)();

        for(int i = 0; i < 4; ++i)
        {
            minX = (std::min)(minX, nearPoints[i].x);
            minY = (std::min)(minY, nearPoints[i].y);
            minZ = (std::min)(minZ, nearPoints[i].z);
            maxX = (std::max)(maxX, nearPoints[i].x);
            maxY = (std::max)(maxY, nearPoints[i].y);
            maxZ = (std::max)(maxZ, nearPoints[i].z);

            minX = (std::min)(minX, farPoints[i].x);
            minY = (std::min)(minY, farPoints[i].y);
            minZ = (std::min)(minZ, farPoints[i].z);
            maxX = (std::max)(maxX, farPoints[i].x);
            maxY = (std::max)(maxY, farPoints[i].y);
            maxZ = (std::max)(maxZ, farPoints[i].z);
        }

        Mat4 shadowProj = Trans4::orthographic(
            minX - 5, maxX + 5, maxY + 5, minY - 5, 300, maxZ + 5);

        return shadowView * shadowProj;
    };

    float distance0 = camera_->GetNearDistance();
    float distance1 = GLOBAL_CONFIG.SHADOW_MAP.distance;
    float distance2 = distance1 + 2 * (distance1 - distance0);
    float distance3 = distance2 + 4 * (distance2 - distance1);

    auto computeHomZLimit = [proj = camera_->GetProjMatrix()](float maxZ)
    {
        Vec4 clipV = Vec4(0, 0, maxZ, 1) * proj;
        return clipV.z;
    };

    VP[0] = constructSingleShadowMapVP(distance0, distance1);
    VP[1] = constructSingleShadowMapVP(distance1, distance2);
    VP[2] = constructSingleShadowMapVP(distance2, distance3);

    homZLimit[0] = computeHomZLimit(distance1);
    homZLimit[1] = computeHomZLimit(distance2);
    homZLimit[2] = computeHomZLimit(distance3);
}

VRPG_GAME_END
