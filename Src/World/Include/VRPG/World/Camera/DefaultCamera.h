#pragma once

#include <VRPG/World/Camera/Camera.h>
#include <VRPG/World/Camera/CursorMoveHistory.h>

VRPG_WORLD_BEGIN

class DefaultCamera : public Camera
{
    CursorMoveHistory cursorHistory_;

public:


};

VRPG_WORLD_END
