#include <VRPG/World/Chunk/ChunkManager.h>
#include <VRPG/World/Chunk/ChunkRenderer.h>

VRPG_WORLD_BEGIN

ChunkManager::ChunkManager(const ChunkManagerParams &params, std::unique_ptr<LandGenerator> landGenerator)
    : params_(params)
{
    loader_ = std::make_unique<ChunkLoader>();
    loader_->Initialize(
        params_.backgroundThreadCount, params_.backgroundPoolSize, std::move(landGenerator));
}

void ChunkManager::SetCentreChunk(int chunkX, int chunkZ)
{
    if(chunkX == centreChunkPosition_.x && chunkZ == centreChunkPosition_.z)
        return;
    centreChunkPosition_.x = chunkX;
    centreChunkPosition_.z = chunkZ;

    // 有哪些需要加载的区块

    int loadXMin = chunkX - params_.loadDistance;
    int loadZMin = chunkZ - params_.loadDistance;
    int loadXMax = chunkX + params_.loadDistance;
    int loadZMax = chunkZ + params_.loadDistance;
    std::vector<ChunkPosition> chunksShouldBeLoad;
    for(int loadX = loadXMin; loadX <= loadXMax; ++loadX)
    {
        for(int loadZ = loadZMin; loadZ <= loadZMax; ++loadZ)
        {
            ChunkPosition loadPosition{ loadX, loadZ };
            if(chunks_.find(loadPosition) == chunks_.end())
                chunksShouldBeLoad.push_back(loadPosition);
        }
    }

    // 将需加载的区块排序后发布，由近及远

    std::sort(chunksShouldBeLoad.begin(), chunksShouldBeLoad.end(),
        [=](const ChunkPosition &lhs, const ChunkPosition &rhs)
    {
        Vec2i CL(lhs.x - chunkX, lhs.z - chunkZ);
        Vec2i CR(rhs.x - chunkX, rhs.z - chunkZ);
        return CL.length_square() < CR.length_square();
    });
    for(auto &position : chunksShouldBeLoad)
        loader_->AddLoadingTask(position);

    // 发布区块卸载任务

    auto shouldBeDestroyed = [
        unloadXMin = chunkX - params_.unloadDistance,
        unloadZMin = chunkZ - params_.unloadDistance,
        unloadXMax = chunkX + params_.unloadDistance,
        unloadZMax = chunkZ + params_.unloadDistance]
        (const ChunkPosition &position)
    {
        return position.x < unloadXMin || position.x > unloadXMax ||
               position.z < unloadZMin || position.z > unloadZMax;
    };
    std::vector<ChunkPosition> chunksShouldBeDestroyed;
    for(auto &pair : chunks_)
    {
        if(shouldBeDestroyed(pair.first))
        {
            chunksShouldBeDestroyed.push_back(pair.first);
            loader_->AddUnloadingTask(std::move(pair.second));
        }
    }
    for(auto &position : chunksShouldBeDestroyed)
        chunks_.erase(position);
}

void ChunkManager::SetBlockID(int blockX, int blockY, int blockZ, BlockID id)
{
    // 分下面几步：
    // 1. 确保被设置的chunk位于内存中，若不，阻塞地加载之
    // 2. 设置id，更新height map，计算哪些方块的天光直接因此而变化
    // 3. 将该消息通知给后台pool
    // 4. 更新光照，其中涉及到的section model均需重新生成
    // 5. 包含被设置的block的section model以及与该block相邻的section model均需重新生成

    auto [ckX, ckZ] = Chunk::BlockToChunk(blockX, blockZ);
    Chunk *chunk = EnsureChunkExists(ckX, ckZ);

    // 设置方块id

    auto [localX, localY, localZ] = Chunk::GlobalToLocal({ blockX, blockY, blockZ });
    chunk->SetID(localX, localY, localZ, id);

    std::queue<Vec3i> blocksWithDirtyLight; // 哪些方块的光照需要重新计算
    blocksWithDirtyLight.push({ blockX, blockY, blockZ });

    // 更新height map

    int oldHeight = chunk->GetHeight(localX, localZ);
    if(blockY > oldHeight && id != BLOCK_ID_VOID)
    {
        // 最大高度提升了

        chunk->SetHeight(localX, localZ, blockY);
        for(int i = oldHeight; i < blockY; ++i)
            blocksWithDirtyLight.push({ blockX, i, blockZ });
    }
    else if(blockY == oldHeight && id == BLOCK_ID_VOID)
    {
        // 最大高度下降了

        int newHeight = blockY;
        while(newHeight >= 0 && chunk->GetID(localX, newHeight, localZ) == BLOCK_ID_VOID)
            --newHeight;

        chunk->SetHeight(localX, newHeight, localZ);
        for(int i = newHeight; i <= blockY; ++i)
            blocksWithDirtyLight.push({ blockX, i, blockZ });
    }

    // 将消息告知loader pool

    loader_->SetChunkBlockDataInPool(blockX, blockY, blockZ, id);

    // 更新光照

    UpdateLight(blocksWithDirtyLight);

    // 将包含该block或与该block相邻的section index放入sectionsWithDirtyModel_

    int sectionX = blockX / CHUNK_SECTION_SIZE_X;
    int sectionY = blockY / CHUNK_SECTION_SIZE_Y;
    int sectionZ = blockZ / CHUNK_SECTION_SIZE_Z;
    int blockInSectionX = blockX % CHUNK_SECTION_SIZE_X;
    int blockInSectionY = blockY % CHUNK_SECTION_SIZE_Y;
    int blockInSectionZ = blockZ % CHUNK_SECTION_SIZE_Z;

    sectionsWithDirtyModel_.insert({ sectionX, sectionY, sectionZ });
    if(blockInSectionX == 0)
        sectionsWithDirtyModel_.insert({ sectionX - 1, sectionY, sectionZ });
    if(blockInSectionX == CHUNK_SECTION_SIZE_X - 1)
        sectionsWithDirtyModel_.insert({ sectionX + 1, sectionY, sectionZ });
    if(blockInSectionZ == 0)
        sectionsWithDirtyModel_.insert({ sectionX, sectionY, sectionZ - 1 });
    if(blockInSectionZ == CHUNK_SECTION_SIZE_Z - 1)
        sectionsWithDirtyModel_.insert({ sectionX, sectionY, sectionZ + 1 });
    if(blockInSectionY == 0 && sectionY > 0)
        sectionsWithDirtyModel_.insert({ sectionX, sectionY - 1, sectionZ });
    if(blockInSectionY == CHUNK_SECTION_SIZE_Y - 1 && sectionY < CHUNK_SECTION_COUNT_Y - 1)
        sectionsWithDirtyModel_.insert({ sectionX, sectionY + 1, sectionZ });
}

BlockID ChunkManager::GetBlockID(int blockX, int blockY, int blockZ)
{
    auto [ckX, ckZ] = Chunk::BlockToChunk(blockX, blockZ);
    auto [localBlockX, localBlockY, localBlockZ] = Chunk::GlobalToLocal({ blockX, blockY, blockZ });
    auto chunk = EnsureChunkExists(ckX, ckZ);
    return chunk->GetID(localBlockX, localBlockY, localBlockZ);
}

BlockBrightness ChunkManager::GetBlockBrightness(int blockX, int blockY, int blockZ)
{
    auto [ckX, ckZ] = Chunk::BlockToChunk(blockX, blockZ);
    auto [localBlockX, localBlockY, localBlockZ] = Chunk::GlobalToLocal({ blockX, blockY, blockZ });
    auto chunk = EnsureChunkExists(ckX, ckZ);
    return chunk->GetBrightness(localBlockX, localBlockY, localBlockZ);
}

void ChunkManager::UpdateChunkData()
{
    auto loadingResults = loader_->GetAllLoadingResults();
    for(auto &chunk : loadingResults)
    {
        ChunkPosition position = chunk->GetPosition();
        auto it = chunks_.find(position);
        if(it == chunks_.end() && !ShouldDestroy(position))
            chunks_[position] = std::move(chunk);
        else
            loader_->AddUnloadingTask(std::move(chunk));
    }
}

bool ChunkManager::UpdateChunkModels()
{
    if(sectionsWithDirtyModel_.empty())
        return false;

    for(auto &secPos : sectionsWithDirtyModel_)
    {
        int ckX = secPos.x / CHUNK_SECTION_COUNT_X;
        int ckZ = secPos.z / CHUNK_SECTION_COUNT_Z;
        int localSecX = secPos.x % CHUNK_SECTION_COUNT_X;
        int localSecZ = secPos.z % CHUNK_SECTION_COUNT_Z;

        auto it = chunks_.find({ ckX, ckZ });
        if(it == chunks_.end())
            continue;

        // EnsureChunkExists可能让it失效，因此在这里暂时记下chunk的地址
        auto chunk = it->second.get();

        const Chunk *neighboringChunks[4];
        neighboringChunks[PositiveX] = EnsureChunkExists(ckX + 1, ckZ);
        neighboringChunks[NegativeX] = EnsureChunkExists(ckX - 1, ckZ);
        neighboringChunks[PositiveZ] = EnsureChunkExists(ckX, ckZ + 1);
        neighboringChunks[NegativeZ] = EnsureChunkExists(ckX, ckZ - 1);

        chunk->RegenerateSectionModel({ localSecX, secPos.y, localSecZ }, neighboringChunks);
    }

    sectionsWithDirtyModel_.clear();
    return true;
}

void ChunkManager::FillRenderer(ChunkRenderer &renderer)
{
    for(auto &pair : chunks_)
    {
        auto &chunk = pair.second;
        if(ShouldRender(chunk->GetPosition()))
        {
            auto &model = chunk->GetChunkModel();
            for(int x = 0; x < CHUNK_SECTION_COUNT_X; ++x)
            {
                for(int z = 0; z < CHUNK_SECTION_COUNT_Z; ++z)
                {
                    for(int y = 0; y < CHUNK_SECTION_COUNT_Y; ++y)
                    {
                        auto &sectionModels = model.sectionModel(x, y, z);
                        for(auto &m : sectionModels->partialModels)
                            renderer.AddPartialSectionModel(m);
                    }
                }
            }
        }
    }
}

Chunk *ChunkManager::EnsureChunkExists(int chunkX, int chunkZ)
{
    if(auto it = chunks_.find({ chunkX, chunkZ }); it != chunks_.end())
        return it->second.get();
    auto newChunk = loader_->LoadChunk({ chunkX, chunkZ });
    auto ret = newChunk.get();
    chunks_[{ chunkX, chunkZ }] = std::move(newChunk);
    return ret;
}

bool ChunkManager::ShouldDestroy(const ChunkPosition &position) const noexcept
{
    int deltaX = position.x - centreChunkPosition_.x;
    int deltaZ = position.z - centreChunkPosition_.z;
    return std::abs(deltaX) > params_.unloadDistance || std::abs(deltaZ) > params_.unloadDistance;
}

bool ChunkManager::ShouldRender(const ChunkPosition &position) const noexcept
{
    int deltaX = position.x - centreChunkPosition_.x;
    int deltaZ = position.z - centreChunkPosition_.z;
    return std::abs(deltaX) <= params_.renderDistance && std::abs(deltaZ) <= params_.renderDistance;
}

void ChunkManager::UpdateLight(std::queue<Vec3i> &blocksQueue)
{
    auto &blockDescMgr = BlockDescriptionManager::GetInstance();

    auto addNeighborToQueue = [&](int x, int y, int z)
    {
        blocksQueue.push({ x - 1, y, z });
        blocksQueue.push({ x + 1, y, z });
        blocksQueue.push({ x, y - 1, z });
        blocksQueue.push({ x, y + 1, z });
        blocksQueue.push({ x, y, z - 1 });
        blocksQueue.push({ x, y, z + 1 });
    };

    while(!blocksQueue.empty())
    {
        auto [x, y, z] = blocksQueue.front();
        blocksQueue.pop();

        auto desc = blockDescMgr.GetBlockDescription(GetBlockID(x, y, z));
        BlockBrightness original = GetBlockBrightness(x, y, z);

        BlockBrightness posX = GetBlockBrightness(x + 1, y, z);
        BlockBrightness posY = GetBlockBrightness(x, y + 1, z);
        BlockBrightness posZ = GetBlockBrightness(x, y, z + 1);
        BlockBrightness negX = GetBlockBrightness(x - 1, y, z);
        BlockBrightness negY = GetBlockBrightness(x, y - 1, z);
        BlockBrightness negZ = GetBlockBrightness(x, y, z - 1);
        BlockBrightness maxNeighborLight = Max(
            Max(posX, Max(posY, posZ)),
            Max(negX, Max(negY, negZ)));

        BlockBrightness directSkyLight;
        if(y > GetHeight_Unchecked(x, z))
            directSkyLight = BLOCK_BRIGHTNESS_SKY;

        BlockBrightness emission = desc->InitialBrightness();
        BlockBrightness attenuation = desc->LightAttenuation();

        BlockBrightness propagated = Max(emission, Max(maxNeighborLight - attenuation, directSkyLight));

        if(propagated != original)
        {
            SetBlockBrightness_Unchecked(x, y, z, propagated);
            addNeighborToQueue(x, y, z);

            int secX = x / CHUNK_SECTION_SIZE_X;
            int secY = y / CHUNK_SECTION_SIZE_Y;
            int secZ = z / CHUNK_SECTION_SIZE_Z;
            sectionsWithDirtyModel_.insert({ secX, secY, secZ });
        }
    }
}

int ChunkManager::GetHeight_Unchecked(int blockX, int blockZ)
{
    int ckX = blockX / CHUNK_SIZE_X;
    int ckZ = blockZ / CHUNK_SIZE_Z;
    int lbX = blockX % CHUNK_SIZE_X;
    int lbZ = blockZ % CHUNK_SIZE_Z;
    return chunks_[{ ckX, ckZ }]->GetHeight(lbX, lbZ);
}

void ChunkManager::SetBlockBrightness_Unchecked(int blockX, int blockY, int blockZ, BlockBrightness brightness)
{
    int ckX = blockX / CHUNK_SIZE_X;
    int ckZ = blockZ / CHUNK_SIZE_Z;
    int lbX = blockX % CHUNK_SIZE_X;
    int lbZ = blockZ % CHUNK_SIZE_Z;
    chunks_[{ ckX, ckZ }]->SetBrightness(lbX, blockY, lbZ, brightness);
}

VRPG_WORLD_END
