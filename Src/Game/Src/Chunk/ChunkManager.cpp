#include <VRPG/Game/Chunk/ChunkManager.h>
#include <VRPG/Game/Chunk/ChunkRenderer.h>

VRPG_GAME_BEGIN

ChunkManager::ChunkManager(const ChunkManagerParams &params, std::unique_ptr<LandGenerator> landGenerator)
    : params_(params)
{
    log_ = spdlog::stdout_color_mt("ChunkManager");

    loader_ = std::make_unique<ChunkLoader>();
    loader_->Initialize(
        params_.backgroundThreadCount, params_.backgroundPoolSize, std::move(landGenerator));

    centreChunkPosition_.x = (std::numeric_limits<int>::max)() - 5;
    centreChunkPosition_.z = (std::numeric_limits<int>::max)() - 5;
}

ChunkManager::~ChunkManager()
{
    for(auto &pair : chunks_)
    {
        loader_->AddUnloadingTask(std::move(pair.second));
    }
    loader_->Destroy();
}

void ChunkManager::SetCentreChunk(const ChunkPosition &chunkPosition)
{
    if(chunkPosition.x == centreChunkPosition_.x && chunkPosition.z == centreChunkPosition_.z)
    {
        return;
    }
    centreChunkPosition_.x = chunkPosition.x;
    centreChunkPosition_.z = chunkPosition.z;

    // 有哪些需要加载的区块

    int loadXMin = chunkPosition.x - params_.loadDistance;
    int loadZMin = chunkPosition.z - params_.loadDistance;
    int loadXMax = chunkPosition.x + params_.loadDistance;
    int loadZMax = chunkPosition.z + params_.loadDistance;
    std::vector<ChunkPosition> chunksShouldBeLoad;
    for(int loadX = loadXMin; loadX <= loadXMax; ++loadX)
    {
        for(int loadZ = loadZMin; loadZ <= loadZMax; ++loadZ)
        {
            ChunkPosition loadPosition{ loadX, loadZ };
            if(chunks_.find(loadPosition) == chunks_.end())
            {
                chunksShouldBeLoad.push_back(loadPosition);
            }
        }
    }

    // 将需加载的区块排序后发布，由近及远

    std::sort(chunksShouldBeLoad.begin(), chunksShouldBeLoad.end(),
        [=](const ChunkPosition &lhs, const ChunkPosition &rhs)
    {
        Vec2i CL(lhs.x - chunkPosition.x, lhs.z - chunkPosition.z);
        Vec2i CR(rhs.x - chunkPosition.x, rhs.z - chunkPosition.z);
        return CL.length_square() < CR.length_square();
    });

    for(auto &position : chunksShouldBeLoad)
    {
        loader_->AddLoadingTask(position);
        log_->trace("add loading task({}, {})", position.x, position.z);
    }

    // 发布区块卸载任务

    auto shouldBeDestroyed = [
        unloadXMin = chunkPosition.x - params_.unloadDistance,
            unloadZMin = chunkPosition.z - params_.unloadDistance,
            unloadXMax = chunkPosition.x + params_.unloadDistance,
            unloadZMax = chunkPosition.z + params_.unloadDistance]
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
    {
        chunks_.erase(position);
    }
}

void ChunkManager::SetBlockID(const Vec3i &globalBlock, BlockID id, BlockOrientation orientation)
{
    // 分下面几步：
    // 1. 确保被设置的chunk位于内存中，若不，阻塞地加载之
    // 2. 设置id，更新height map，计算哪些方块的天光直接因此而变化
    // 3. 将该消息通知给后台pool
    // 4. 更新光照，其中涉及到的section model均需重新生成
    // 5. 包含被设置的block的section model以及与该block相邻的section model均需重新生成

    if(globalBlock.y < 0 || globalBlock.y >= CHUNK_SIZE_Y)
    {
        return;
    }

    auto [ckPos, blkPos] = DecomposeGlobalBlockByChunk(globalBlock);
    Chunk *chunk = EnsureChunkExists(ckPos.x, ckPos.z);

    // 设置方块id

    chunk->SetID(blkPos, id, orientation);

    // 更新height map

    int oldHeight = chunk->GetHeight(blkPos.x, blkPos.z);
    if(blkPos.y > oldHeight &&id != BLOCK_ID_VOID)
    {
        // 最大高度提升了

        chunk->SetHeight(blkPos.x, blkPos.z, blkPos.y);
        for(int i = oldHeight; i < globalBlock.y; ++i)
        {
            blocksWithDirtyLight_.push({ globalBlock.x, i, globalBlock.z });
        }
    }
    else if(globalBlock.y == oldHeight && id == BLOCK_ID_VOID)
    {
        // 最大高度下降了

        int newHeight = globalBlock.y;
        while(newHeight >= 0 && chunk->GetID({ blkPos.x, newHeight, blkPos.z }) == BLOCK_ID_VOID)
        {
            --newHeight;
        }

        chunk->SetHeight(blkPos.x, blkPos.z, newHeight);
        for(int i = newHeight; i <= globalBlock.y; ++i)
        {
            blocksWithDirtyLight_.push({ globalBlock.x, i, globalBlock.z });
        }
    }

    // 将消息告知loader pool

    loader_->SetChunkBlockDataInPool(globalBlock.x, globalBlock.y, globalBlock.z, id, orientation);

    // 更新光照和section model

    blocksWithDirtyLight_.push(globalBlock);
    MakeNeighborSectionsDirty(globalBlock);
}

void ChunkManager::SetBlockID(const Vec3i &globalBlock, uint16_t id, BlockOrientation orientation, BlockExtraData extraData)
{
    if(globalBlock.y < 0 || globalBlock.y >= CHUNK_SIZE_Y)
    {
        return;
    }
    SetBlockID(globalBlock, id, orientation);
    *GetExtraData(globalBlock) = std::move(extraData);
}

BlockID ChunkManager::GetBlockID(const Vec3i &globalBlock)
{
    if(globalBlock.y < 0 || globalBlock.y >= CHUNK_SIZE_Y)
    {
        return BLOCK_ID_VOID;
    }
    auto [ckPos, blkPos] = DecomposeGlobalBlockByChunk(globalBlock);
    auto chunk = EnsureChunkExists(ckPos.x, ckPos.z);
    return chunk->GetID(blkPos);
}

const BlockDescription *ChunkManager::GetBlockDesc(const Vec3i &globalBlock)
{
    return BlockDescManager::GetInstance().GetBlockDescription(GetBlockID(globalBlock));
}

BlockExtraData *ChunkManager::GetExtraData(const Vec3i &globalBlock)
{
    auto [ckPos, blkPos] = DecomposeGlobalBlockByChunk(globalBlock);
    auto chunk = EnsureChunkExists(ckPos.x, ckPos.z);
    return chunk->GetExtraData(blkPos);
}

BlockBrightness ChunkManager::GetBlockBrightness(const Vec3i &globalBlock)
{
    if(globalBlock.y < 0 || globalBlock.y >= CHUNK_SIZE_Y)
    {
        return BLOCK_BRIGHTNESS_MIN;
    }
    auto [ckPos, blkPos] = DecomposeGlobalBlockByChunk(globalBlock);
    auto chunk = EnsureChunkExists(ckPos.x, ckPos.z);
    return chunk->GetBrightness(blkPos);
}

BlockInstance ChunkManager::GetBlock(const Vec3i &globalBlock)
{
    if(globalBlock.y < 0 || globalBlock.y >= CHUNK_SIZE_Y)
    {
        return BlockInstance{
                nullptr,
            nullptr,
            BLOCK_BRIGHTNESS_MIN,
            BlockOrientation()
        };
    }
    auto [ckPos, blkPos] = DecomposeGlobalBlockByChunk(globalBlock);
    auto chunk = EnsureChunkExists(ckPos.x, ckPos.z);
    return chunk->GetBlock(blkPos);
}

bool ChunkManager::FindClosestIntersectedBlock(
    const Vec3 &o, const Vec3 &d, float maxDistance, Vec3i *pickedBlock, Direction *pickedFace,
    const std::function<bool(const BlockDescription*)> &blockFilter)
{
    auto &blockDescMgr = BlockDescManager::GetInstance();
    Vec3i lastBlockPosition = Vec3i(std::numeric_limits<Vec3i::elem_t>::lowest());

    constexpr float STEP = 0.01f;
    float t = 0;

    while(t <= maxDistance)
    {
        Vec3 p = o + t * d;
        t += STEP;
        Vec3i blockPosition = p.map([](float c) { return int(std::floor(c)); });
        if(blockPosition == lastBlockPosition)
        {
            continue;
        }
        lastBlockPosition = blockPosition;

        BlockID id = GetBlockID(blockPosition);
        if(id == BLOCK_ID_VOID)
        {
            continue;
        }
        const BlockDescription *desc = blockDescMgr.GetBlockDescription(id);

        // IMPROVE: block orientation
        Vec3 localStart = {
            o.x - blockPosition.x,
            o.y - blockPosition.y,
            o.z - blockPosition.z
        };

        if(desc->RayIntersect(localStart, d, 0, maxDistance, pickedFace) && blockFilter(desc))
        {
            if(pickedBlock)
            {
                *pickedBlock = blockPosition;
            }
            return true;
        }
    }

    return false;
}

bool ChunkManager::UpdateChunkData()
{
    auto loadingResults = loader_->GetAllLoadingResults();
    bool ret = !loadingResults.empty();

    for(auto &chunk : loadingResults)
    {
        ChunkPosition position = chunk->GetPosition();
        auto it = chunks_.find(position);
        if(it == chunks_.end() && !ShouldDestroy(position))
        {
            chunks_[position] = std::move(chunk);
        }
        else
        {
            loader_->AddUnloadingTask(std::move(chunk));
        }
    }

    return ret;
}

void ChunkManager::UpdateLight()
{
    UpdateLight(blocksWithDirtyLight_);
}

bool ChunkManager::UpdateChunkModels()
{
    if(sectionsWithDirtyModel_.empty())
    {
        return false;
    }

    for(auto &secPos : sectionsWithDirtyModel_)
    {
        auto [ckPos, secInCk] = DecomposeGlobalSectionByChunk(secPos);

        auto it = chunks_.find({ ckPos.x, ckPos.z});
        if(it == chunks_.end())
        {
            continue;
        }

        // EnsureChunkExists可能让it失效，因此在这里暂时记下chunk的地址
        auto chunk = it->second.get();

        const Chunk *neighboringChunks[3][3];
        neighboringChunks[0][0] = EnsureChunkExists(ckPos.x - 1, ckPos.z - 1);
        neighboringChunks[0][1] = EnsureChunkExists(ckPos.x - 1, ckPos.z);
        neighboringChunks[0][2] = EnsureChunkExists(ckPos.x - 1, ckPos.z + 1);
        neighboringChunks[1][0] = EnsureChunkExists(ckPos.x, ckPos.z - 1);
        neighboringChunks[1][1] = chunk;
        neighboringChunks[1][2] = EnsureChunkExists(ckPos.x, ckPos.z + 1);
        neighboringChunks[2][0] = EnsureChunkExists(ckPos.x + 1, ckPos.z - 1);
        neighboringChunks[2][1] = EnsureChunkExists(ckPos.x + 1, ckPos.z);
        neighboringChunks[2][2] = EnsureChunkExists(ckPos.x + 1, ckPos.z + 1);

        chunk->RegenerateSectionModel({ secInCk.x, secInCk.y, secInCk.z }, neighboringChunks);
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
                        {
                            renderer.AddPartialSectionModel(m);
                        }
                    }
                }
            }
        }
    }
}

Chunk *ChunkManager::EnsureChunkExists(int chunkX, int chunkZ)
{
    if(auto it = chunks_.find({ chunkX, chunkZ }); it != chunks_.end())
    {
        return it->second.get();
    }

    loader_->AddLoadingTask({ chunkX, chunkZ });
    for(;;)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        UpdateChunkData();

        auto it = chunks_.find({ chunkX, chunkZ });
        if(it != chunks_.end())
        {
            return it->second.get();
        }
    }
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
    auto &blockDescMgr = BlockDescManager::GetInstance();

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
        Vec3i pos = blocksQueue.front();
        auto [x, y, z] = pos;
        blocksQueue.pop();

        if(y < 0 || y >= CHUNK_SIZE_Y)
        {
            continue;
        }

        auto desc = blockDescMgr.GetBlockDescription(GetBlockID(pos));
        BlockBrightness original = GetBlockBrightness(pos);

        BlockBrightness posX = GetBlockBrightness({ x + 1, y, z });
        BlockBrightness posY = GetBlockBrightness({ x, y + 1, z });
        BlockBrightness posZ = GetBlockBrightness({ x, y, z + 1 });
        BlockBrightness negX = GetBlockBrightness({ x - 1, y, z });
        BlockBrightness negY = GetBlockBrightness({ x, y - 1, z });
        BlockBrightness negZ = GetBlockBrightness({ x, y, z - 1 });
        BlockBrightness maxNeighborLight = Max(
            Max(posX, Max(posY, posZ)),
            Max(negX, Max(negY, negZ)));

        BlockBrightness directSkyLight;
        if(y > GetHeight_Unchecked(x, z))
        {
            directSkyLight = BLOCK_BRIGHTNESS_SKY;
        }

        BlockBrightness emission = desc->InitialBrightness();
        BlockBrightness attenuation = desc->LightAttenuation();

        BlockBrightness propagated = Max(emission, Max(maxNeighborLight - attenuation, directSkyLight));

        if(propagated != original)
        {
            SetBlockBrightness_Unchecked(pos, propagated);
            addNeighborToQueue(x, y, z);
            MakeNeighborSectionsDirty(pos);
        }
    }
}

int ChunkManager::GetHeight_Unchecked(int blockX, int blockZ)
{
    auto [ck, lb] = DecomposeGlobalBlockByChunk({ blockX, 0, blockZ });
    return chunks_[ck]->GetHeight(lb.x, lb.z);
}

void ChunkManager::SetBlockBrightness_Unchecked(const Vec3i &globalBlock, BlockBrightness brightness)
{
    auto [ck, lb] = DecomposeGlobalBlockByChunk(globalBlock);
    chunks_[ck]->SetBrightness(lb, brightness);
}

void ChunkManager::MakeNeighborSectionsDirty(const Vec3i &globalBlockPosition)
{
    auto [sectionX, sectionY, sectionZ] = GlobalBlockToGlobalSection(globalBlockPosition);
    auto [blockInSectionX, blockInSectionY, blockInSectionZ] = GlobalBlockToBlockInSection(globalBlockPosition);

    sectionsWithDirtyModel_.insert({ sectionX, sectionY, sectionZ });

    bool lowerX  = blockInSectionX == 0;
    bool lowerY  = blockInSectionY == 0 && sectionY > 0;
    bool lowerZ  = blockInSectionZ == 0;
    bool higherX = blockInSectionX == CHUNK_SECTION_SIZE_X - 1;
    bool higherY = blockInSectionY == CHUNK_SECTION_SIZE_Y - 1 && sectionY < CHUNK_SECTION_COUNT_Y - 1;
    bool higherZ = blockInSectionZ == CHUNK_SECTION_SIZE_Z - 1;

    if(lowerX)  sectionsWithDirtyModel_.insert({ sectionX - 1, sectionY, sectionZ });
    if(lowerY)  sectionsWithDirtyModel_.insert({ sectionX, sectionY - 1, sectionZ });
    if(lowerZ)  sectionsWithDirtyModel_.insert({ sectionX, sectionY, sectionZ - 1 });
    if(higherX) sectionsWithDirtyModel_.insert({ sectionX + 1, sectionY, sectionZ });
    if(higherY) sectionsWithDirtyModel_.insert({ sectionX, sectionY + 1, sectionZ });
    if(higherZ) sectionsWithDirtyModel_.insert({ sectionX, sectionY, sectionZ + 1 });

    if(lowerX  && lowerY)  sectionsWithDirtyModel_.insert({ sectionX - 1, sectionY - 1, sectionZ });
    if(lowerX  && higherY) sectionsWithDirtyModel_.insert({ sectionX - 1, sectionY + 1, sectionZ });
    if(higherX && lowerY)  sectionsWithDirtyModel_.insert({ sectionX + 1, sectionY - 1, sectionZ });
    if(higherX && higherY) sectionsWithDirtyModel_.insert({ sectionX + 1, sectionY + 1, sectionZ });

    if(lowerY  && lowerZ)  sectionsWithDirtyModel_.insert({ sectionX, sectionY - 1, sectionZ - 1 });
    if(lowerY  && higherZ) sectionsWithDirtyModel_.insert({ sectionX, sectionY - 1, sectionZ + 1 });
    if(higherY && lowerZ)  sectionsWithDirtyModel_.insert({ sectionX, sectionY + 1, sectionZ - 1 });
    if(higherY && higherZ) sectionsWithDirtyModel_.insert({ sectionX, sectionY + 1, sectionZ + 1 });

    if(lowerX  && lowerZ)  sectionsWithDirtyModel_.insert({ sectionX - 1, sectionY, sectionZ - 1 });
    if(lowerX  && higherZ) sectionsWithDirtyModel_.insert({ sectionX - 1, sectionY, sectionZ + 1 });
    if(higherX && lowerZ)  sectionsWithDirtyModel_.insert({ sectionX + 1, sectionY, sectionZ - 1 });
    if(higherX && higherZ) sectionsWithDirtyModel_.insert({ sectionX + 1, sectionY, sectionZ + 1 });

    if(lowerX  && lowerY  && lowerZ)  sectionsWithDirtyModel_.insert({ sectionX - 1, sectionY - 1, sectionZ - 1 });
    if(lowerX  && lowerY  && higherZ) sectionsWithDirtyModel_.insert({ sectionX - 1, sectionY - 1, sectionZ + 1 });
    if(lowerX  && higherY && lowerZ)  sectionsWithDirtyModel_.insert({ sectionX - 1, sectionY + 1, sectionZ - 1 });
    if(lowerX  && higherY && higherZ) sectionsWithDirtyModel_.insert({ sectionX - 1, sectionY + 1, sectionZ + 1 });
    if(higherX && lowerY  && lowerZ)  sectionsWithDirtyModel_.insert({ sectionX + 1, sectionY - 1, sectionZ - 1 });
    if(higherX && lowerY  && higherZ) sectionsWithDirtyModel_.insert({ sectionX + 1, sectionY - 1, sectionZ + 1 });
    if(higherX && higherY && lowerZ)  sectionsWithDirtyModel_.insert({ sectionX + 1, sectionY + 1, sectionZ - 1 });
    if(higherX && higherY && higherZ) sectionsWithDirtyModel_.insert({ sectionX + 1, sectionY + 1, sectionZ + 1 });
}

VRPG_GAME_END
