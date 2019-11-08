#pragma once

#include <agz/utility/time.h>

#include <VRPG/World/Camera/DefaultCamera.h>
#include <VRPG/World/Chunk/ChunkManager.h>
#include <VRPG/World/Chunk/ChunkRenderer.h>
#include <VRPG/World/Misc/Crosshair.h>

VRPG_WORLD_BEGIN

class Game : public agz::misc::uncopyable_t
{
public:

    explicit Game(Base::Window *window);

    void Run();

private:

    using us = std::chrono::microseconds;

    void Initialize();

    void PlayerTick(float deltaT);

    void WorldTick();

    void Render();

    void Destroy();

    Base::Window               *window_;
    Base::KeyboardEventManager *keyboard_;
    Base::MouseEventManager    *mouse_;

    std::unique_ptr<Immediate2D> imm2D_;
    std::unique_ptr<Crosshair> crosshairPainter_;

    std::unique_ptr<DefaultCamera> camera_;
    std::unique_ptr<ChunkRenderer> chunkRenderer_;
    std::unique_ptr<ChunkManager> chunkManager_;

    us worldTickInterval_;

    agz::time::fps_counter_t fpsCounter_;

    bool exitMainloop_;
};

VRPG_WORLD_END
