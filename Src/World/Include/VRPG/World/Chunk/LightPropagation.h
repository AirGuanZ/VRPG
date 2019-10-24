#pragma once

#include <VRPG/World/Chunk/Chunk.h>

VRPG_WORLD_BEGIN

void PropagateLightForCentreChunk(Chunk *(&chunks)[3][3]);

VRPG_WORLD_END
