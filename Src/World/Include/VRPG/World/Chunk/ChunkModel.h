#pragma once

#include <memory>

#include <VRPG/World/Camera/Camera.h>
#include <VRPG/World/Chunk/Common.h>

VRPG_WORLD_BEGIN

/*
    每种BlockEffect对应一种对应的ChunkDataBuilder
    Build一个Chunk的RenderingData前先把每种ChunkDataBuilder准备好
    BlockDescription自己根据BlockEffectID拿到对应的ChunkDataBuilder，这里需要dynamic_cast
*/

/*
区块模型层级：
    一个chunk model包含 CHUNK_SECTION_COUNT_X * CHUNK_SECTION_COUNT_Z * CHUNK_SECTION_COUNT_Y个section model
    一个section model由一系列partial section model构成
    一个partial section model对应一种block effect
*/

class BlockEffect;

class PartialSectionModel
{
public:

    virtual ~PartialSectionModel() = default;

    virtual void Render(const Camera &camera) const = 0;

    virtual const BlockEffect *GetBlockEffect() const noexcept = 0;
};

class PartialSectionModelBuilder
{
public:

    virtual ~PartialSectionModelBuilder() = default;

    virtual std::shared_ptr<const PartialSectionModel> Build() const = 0;
};

class SectionModel
{
public:

    std::vector<std::shared_ptr<const PartialSectionModel>> partialModels;
};

class ChunkModel
{
    std::unique_ptr<const SectionModel> sectionModels_
        [CHUNK_SECTION_COUNT_X][CHUNK_SECTION_COUNT_Z][CHUNK_SECTION_COUNT_Y];

public:

    std::unique_ptr<const SectionModel> &sectionModel(int x, int y, int z) noexcept
    {
        return sectionModels_[x][z][y];
    }

    const std::unique_ptr<const SectionModel> &sectionModel(int x, int y, int z) const noexcept
    {
        return sectionModels_[x][z][y];
    }

    std::unique_ptr<const SectionModel> &sectionModel(const Vec3i &sectionIndex) noexcept
    {
        return sectionModel(sectionIndex.x, sectionIndex.y, sectionIndex.z);
    }

    const std::unique_ptr<const SectionModel> &sectionModel(const Vec3i &sectionIndex) const noexcept
    {
        return sectionModel(sectionIndex.x, sectionIndex.y, sectionIndex.z);
    }
};

VRPG_WORLD_END
