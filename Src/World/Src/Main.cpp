#include <iostream>

#include <agz/utility/time.h>

#include <ImGui/imgui.h>

#include <VRPG/Base/Base.h>

#include <VRPG/World/Camera/DefaultCamera.h>
#include <VRPG/World/Chunk/ChunkManager.h>
#include <VRPG/World/Chunk/ChunkRenderer.h>
#include <VRPG/World/Block/BuiltinBlock/BuiltinBlock.h>
#include <VRPG/World/Block/BlockDescription.h>
#include <VRPG/World/Block/BlockEffect.h>
#include <VRPG/World/Land/FlatLandGenerator.h>
#include <VRPG/World/Misc/Crosshair.h>
#include <VRPG/World/Utility/ScalarHistory.h>

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

    KeyboardEventManager *keyboard = window.GetKeyboard();
    MouseEventManager *mouse = window.GetMouse();
	
	spdlog::info("initialize builtin block manager");
	BuiltinBlockTypeManager::GetInstance().RegisterBuiltinBlockTypes();
	AGZ_SCOPE_GUARD({
		spdlog::info("destroy builtin block manager");
		BuiltinBlockTypeManager::GetInstance().Clear();
		BlockEffectManager::GetInstance().Clear();
		BlockDescriptionManager::GetInstance().Clear();
	});

    ChunkManagerParams params = {};
    params.loadDistance          = 6;
    params.backgroundPoolSize    = 200;
    params.backgroundThreadCount = 1;
    params.renderDistance        = 5;
    params.unloadDistance        = 8;
    ChunkManager chunkMgr(params, std::make_unique<FlatLandGenerator>(20));

    DefaultCamera camera;
    camera.SetMoveSpeed(7);
    camera.SetViewSpeed(0.003f);
    camera.SetWOverH(window.GetClientAspectRatio());
    camera.SetPosition({ 0, 30, 0 });
    camera.SetClipDistance(0.1f, 1000.0f);

    spdlog::info("start mainloop");

    auto renderer = std::make_shared<ChunkRenderer>();

    agz::time::fps_counter_t fps;

    mouse->ShowCursor(false);
    mouse->SetCursorLock(true, window.GetClientSizeX() / 2, window.GetClientSizeY() / 2);
    mouse->UpdatePosition();

    ScalarHistory deltaTHistory(4);

    bool isLBPressed = false;
    bool isLBDown    = false;

    Immediate2D imm2D;
    imm2D.SetFramebufferSize({ window.GetClientSizeX(), window.GetClientSizeY() });
    WindowResizeHandler windowResizeHandler([&](const WindowResizeEvent &e)
    {
        imm2D.SetFramebufferSize({ e.newClientWidth, e.newClientHeight });
    });
    Crosshair crosshair;

    while(!window.GetCloseFlag())
    {
        float deltaT = fps.elasped_microseconds() / 1000.0f;
        deltaTHistory.Update(deltaT);
        deltaT = deltaTHistory.MeanValue();

        window.DoEvents(true, true);
        window.WaitForFocus();

        window.ImGuiNewFrame();

        if(keyboard->IsKeyPressed(KEY_ESCAPE))
            window.SetCloseFlag(true);

        DefaultCamera::Input cameraInput;
        cameraInput.front = keyboard->IsKeyPressed('W');
        cameraInput.back  = keyboard->IsKeyPressed('S');
        cameraInput.left  = keyboard->IsKeyPressed('A');
        cameraInput.right = keyboard->IsKeyPressed('D');
        cameraInput.up    = keyboard->IsKeyPressed(KEY_SPACE);
        cameraInput.down  = keyboard->IsKeyPressed(KEY_LSHIFT);

        cameraInput.relativeCursorX = float(mouse->GetRelativeCursorPositionX());
        cameraInput.relativeCursorY = float(mouse->GetRelativeCursorPositionY());

        camera.Update(cameraInput, deltaT);

        int cameraBlockX = int(camera.GetPosition().x), cameraBlockZ = int(camera.GetPosition().z);
        chunkMgr.SetCentreChunk(GlobalBlockToChunk(cameraBlockX, cameraBlockZ));

        isLBDown = !isLBPressed && mouse->IsMouseButtonPressed(MouseButton::Left);
        isLBPressed = mouse->IsMouseButtonPressed(MouseButton::Left);

        Vec3i pickedBlockPosition;
        BlockID pickedBlockID = BLOCK_ID_VOID;
        if(chunkMgr.FindClosestIntersectedBlock(camera.GetPosition(), camera.GetDirection(), 8.0f, &pickedBlockPosition))
        {
            if(isLBDown)
                chunkMgr.SetBlockID(pickedBlockPosition, BLOCK_ID_VOID, {});
            else
                pickedBlockID = chunkMgr.GetBlockID(pickedBlockPosition);
        }

        bool needToGenerateRenderer = false;
        needToGenerateRenderer |= chunkMgr.UpdateChunkData();
        needToGenerateRenderer |= chunkMgr.UpdateChunkModels();

        if(needToGenerateRenderer)
        {
            renderer->Clear();
            chunkMgr.FillRenderer(*renderer);
            renderer->Done();
        }

        constexpr auto PANEL_FLAG =
            ImGuiWindowFlags_NoMove             |
            ImGuiWindowFlags_NoTitleBar         |
            ImGuiWindowFlags_NoResize           |
            ImGuiWindowFlags_AlwaysAutoResize   |
            ImGuiWindowFlags_NoSavedSettings    |
            ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoNav;
        if(ImGui::Begin("debug", nullptr, PANEL_FLAG))
        {
            ImGui::Text("fps: %d", fps.fps());

            auto camPos = camera.GetPosition();
            ImGui::Text("pos: %f, %f, %f", camPos.x, camPos.y, camPos.z);

            auto camDir = camera.GetDirection();
            ImGui::Text("dir: %f, %f, %f", camDir.x, camDir.y, camDir.z);

            auto pickedBlockDesc = BlockDescriptionManager::GetInstance().GetBlockDescription(pickedBlockID);
            ImGui::Text("picked: %s", pickedBlockDesc->GetName());
        }
        ImGui::End();

        Vec4 backgroundColor = Vec4(0.7f, 1, 1, 0).map([](float x) { return std::pow(x, 1 / 2.2f); });
        window.ClearDefaultRenderTarget(&backgroundColor[0]);
        window.ClearDefaultDepthStencil();

        renderer->Render({ &camera, Vec3(1) });

        crosshair.Draw(imm2D);

        window.ImGuiRender();
        window.SwapBuffers();
        fps.frame_end();
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
