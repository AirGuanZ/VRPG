#pragma once

#include <agz/utility/time.h>

#include <VRPG/Game/Camera/DefaultCamera.h>
#include <VRPG/Game/Chunk/ChunkManager.h>
#include <VRPG/Game/Chunk/ChunkRenderer.h>
#include <VRPG/Game/Misc/Crosshair.h>
#include <VRPG/Game/Utility/KeyStateTracker.h>
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

    void PlayerTick(float deltaT);

    void WorldTick();

    void ChunkTick();

    void Render(int fps);

    void Destroy();

    Mat4 ConstructShadowMapVP() const;

    Base::Window               *window_;
    Base::KeyboardEventManager *keyboard_;
    Base::MouseEventManager    *mouse_;

    MouseButtonStateTracker lbState_;
    MouseButtonStateTracker rbState_;

    std::unique_ptr<Immediate2D> imm2D_;
    std::unique_ptr<Crosshair> crosshairPainter_;

    std::unique_ptr<DefaultCamera> camera_;
    std::unique_ptr<Base::ShadowMap> shadowMap_;

    std::unique_ptr<ChunkRenderer> chunkRenderer_;
    std::unique_ptr<ChunkManager> chunkManager_;

    std::unique_ptr<BlockUpdaterManager> blockUpdaterManager_;
};

VRPG_GAME_END
