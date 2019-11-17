#pragma once

#include <agz/utility/time.h>

#include <VRPG/Game/Camera/DefaultCamera.h>
#include <VRPG/Game/Chunk/ChunkManager.h>
#include <VRPG/Game/Chunk/ChunkRenderer.h>
#include <VRPG/Game/Misc/CascadeShadowMapping.h>
#include <VRPG/Game/Misc/ChosenWireframe.h>
#include <VRPG/Game/Misc/Crosshair.h>
#include <VRPG/Game/World/BlockUpdater.h>

VRPG_GAME_BEGIN

class Game : public agz::misc::uncopyable_t
{
public:

    explicit Game(Base::Window *window);

    void Run();

private:

    using us = std::chrono::microseconds;

    static constexpr us WORLD_TICK_INTERVAL = us(50 * 1000);

    void Initialize();

    void Destroy();

    void PlayerTick(float deltaT);

    void WorldTick();

    void ChunkTick();

    void Render(int fps);

    void HideCursor();

    void UpdateCamera(float deltaT);

    void UpdateCentreChunk();

    Base::Window               *window_;
    Base::KeyboardEventManager *keyboard_;
    Base::MouseEventManager    *mouse_;

    std::unique_ptr<Immediate2D> imm2D_;
    std::unique_ptr<Crosshair>   crosshairPainter_;

    std::unique_ptr<CascadeShadowMapping> CSM_;

    std::unique_ptr<DefaultCamera> camera_;
    std::unique_ptr<ChunkRenderer> chunkRenderer_;
    std::unique_ptr<ChunkManager>  chunkManager_;

    std::unique_ptr<BlockUpdaterManager> blockUpdaterManager_;

    std::unique_ptr<ChosenWireframeRenderer> chosenBlockWireframeRenderer_;
    std::optional<Vec3i> chosenBlockPosition_;
};

VRPG_GAME_END
