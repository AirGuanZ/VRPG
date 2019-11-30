#pragma once

#include <agz/utility/time.h>

#include <VRPG/Game/Mesh/DiffuseSolidMesh.h>
#include <VRPG/Game/Misc/CascadeShadowMapping.h>
#include <VRPG/Game/Misc/ChosenWireframe.h>
#include <VRPG/Game/Misc/Crosshair.h>
#include <VRPG/Game/Player/Player.h>
#include <VRPG/Game/World/BlockUpdater/BlockUpdater.h>
#include <VRPG/Game/World/Chunk/ChunkManager.h>
#include <VRPG/Game/World/Chunk/ChunkRenderer.h>

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

    void UpdatePlayer(float deltaT);

    void UpdateCentreChunk();

    Base::Window               *window_;
    Base::KeyboardEventManager *keyboard_;
    Base::MouseEventManager    *mouse_;

    std::unique_ptr<Immediate2D> imm2D_;
    std::unique_ptr<Crosshair>   crosshairPainter_;

    std::unique_ptr<CascadeShadowMapping> CSM_;

    std::unique_ptr<Player>        player_;
    std::unique_ptr<ChunkRenderer> chunkRenderer_;
    std::unique_ptr<ChunkManager>  chunkManager_;

    std::unique_ptr<BlockUpdaterManager> blockUpdaterManager_;

    std::unique_ptr<ChosenWireframeRenderer> chosenBlockWireframeRenderer_;
    std::optional<Vec3i>                     chosenBlockPosition_;

    std::shared_ptr<DiffuseSolidMeshEffect> meshEffect_;
    std::unique_ptr<DiffuseSolidMesh> mesh_;
};

VRPG_GAME_END
