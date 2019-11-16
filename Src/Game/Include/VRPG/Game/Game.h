#pragma once

#include <agz/utility/time.h>

#include <VRPG/Game/Camera/DefaultCamera.h>
#include <VRPG/Game/Chunk/ChunkManager.h>
#include <VRPG/Game/Chunk/ChunkRenderer.h>
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

    void PlayerTick(float deltaT);

    void WorldTick();

    void ChunkTick();

    void Render(int fps);

    void Destroy();

    void HideCursor();

    void UpdateCentreChunk();

    void ConstructShadowMapVP(Mat4 VP[3], float homZLimit[3]) const;

    Base::Window               *window_;
    Base::KeyboardEventManager *keyboard_;
    Base::MouseEventManager    *mouse_;

    std::unique_ptr<Immediate2D> imm2D_;
    std::unique_ptr<Crosshair> crosshairPainter_;

    std::unique_ptr<DefaultCamera> camera_;
    std::unique_ptr<Base::ShadowMap> nearShadowMap_;
    std::unique_ptr<Base::ShadowMap> middleShadowMap_;
    std::unique_ptr<Base::ShadowMap> farShadowMap_;

    std::unique_ptr<ChunkRenderer> chunkRenderer_;
    std::unique_ptr<ChunkManager> chunkManager_;

    std::unique_ptr<BlockUpdaterManager> blockUpdaterManager_;
};

VRPG_GAME_END
