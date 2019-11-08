#pragma once

#include <VRPG/Game/Chunk/Chunk.h>

VRPG_GAME_BEGIN

void PropagateLightForCentreChunk(Chunk *(&chunks)[3][3]);

VRPG_GAME_END
