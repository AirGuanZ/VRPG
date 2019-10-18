#pragma once

#include <memory>

#include <VRPG/World/Common.h>

VRPG_WORLD_BEGIN

/*
每种BlockEffect对应一种对应的ChunkDataBuilder
Build一个Chunk的RenderingData前先把每种ChunkDataBuilder准备好
BlockDescription自己根据BlockEffectID拿到对应的ChunkDataBuilder，这里需要dynamic_cast
*/

class BlockEffect;

class ChunkModel
{
public:

    virtual ~ChunkModel() = default;

    virtual void Render() const = 0;

    virtual const BlockEffect *GetBlockEffect() const noexcept = 0;
};

class ChunkModelBuilder
{
public:

    virtual ~ChunkModelBuilder() = default;

    virtual std::shared_ptr<const ChunkModel> Build() const = 0;
};

VRPG_WORLD_END
