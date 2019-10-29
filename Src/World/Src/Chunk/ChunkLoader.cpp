#include <agz/utility/misc.h>

#include <VRPG/World/Chunk/ChunkLoader.h>
#include <VRPG/World/Chunk/LightPropagation.h>

VRPG_WORLD_BEGIN

namespace
{
    size_t PositionToThreadIndex(const ChunkPosition &position, size_t threadCount)
    {
        size_t positionHash = std::hash<ChunkPosition>()(position);
        return positionHash % threadCount;
    }
}

ChunkLoader::ChunkLoader()
{
    skipLoading_ = false;
    log_ = spdlog::stdout_color_mt("ChunkLoader");
}

ChunkLoader::~ChunkLoader()
{
    assert(!IsAvailable());
}

void ChunkLoader::Initialize(int threadCount, int poolSize, std::unique_ptr<LandGenerator> landGenerator)
{
    assert(!IsAvailable());
    assert(threadCount > 0 && poolSize > 0 && landGenerator);

    blockDataPool_ = std::make_unique<ChunkBlockDataPool>(poolSize);
    landGenerator_ = std::move(landGenerator);

    perThreadData_.reset(new PerThreadData[threadCount]);
    threads_.reserve(threadCount);
    for(int i = 0; i < threadCount; ++i)
        threads_.emplace_back(&WorkerFunc, this, &perThreadData_[i]);

    loadingResults_ = std::make_unique<std::queue<std::unique_ptr<Chunk>>>();
}

bool ChunkLoader::IsAvailable() const noexcept
{
	return !threads_.empty();
}

void ChunkLoader::Destroy()
{
    if(!IsAvailable())
        return;

    skipLoading_ = true;
    for(size_t i = 0; i < threads_.size(); ++i)
        perThreadData_[i].taskQueue.Stop();

    for(auto &t : threads_)
    {
        assert(t.joinable());
        t.join();
    }

    threads_.clear();
    perThreadData_.reset();

    blockDataPool_.reset();
    landGenerator_.reset();

    std::lock_guard lk(loadingResultsMutex_);
    loadingResults_.reset();
}

void ChunkLoader::AddLoadingTask(const ChunkPosition &position)
{
    assert(IsAvailable());
    size_t threadIndex = PositionToThreadIndex(position, threads_.size());
    perThreadData_[threadIndex].taskQueue.AddLoadingTask(position);
}

void ChunkLoader::AddUnloadingTask(std::unique_ptr<Chunk> &&chunk)
{
    assert(IsAvailable() && chunk);
    size_t threadIndex = PositionToThreadIndex(chunk->GetPosition(), threads_.size());
    perThreadData_[threadIndex].taskQueue.AddUnloadingTask(std::move(chunk));
}

std::vector<std::unique_ptr<Chunk>> ChunkLoader::GetAllLoadingResults()
{
    std::vector<std::unique_ptr<Chunk>> ret;
    {
        std::lock_guard lk(loadingResultsMutex_);
        ret.reserve(loadingResults_->size());
        while(!loadingResults_->empty())
        {
            ret.push_back(std::move(loadingResults_->front()));
            loadingResults_->pop();
        }
    }
    return ret;
}

void ChunkLoader::AddLoadingResult(std::unique_ptr<Chunk> &&loadedChunk)
{
    assert(loadedChunk);
    std::lock_guard lk(loadingResultsMutex_);
    loadingResults_->push(std::move(loadedChunk));
}

std::unique_ptr<Chunk> ChunkLoader::LoadChunk(const ChunkPosition &position)
{
    // 生成/加载方块数据
    // 池子中的数据可能是过时的，因此这里强制重新生成
    // 池子里的数据实际上只是用来计算光照而已

    auto chunk = std::make_unique<Chunk>(position);
    landGenerator_->Generate(position, &chunk->GetBlockData());
    blockDataPool_->TryToAddChunkBlockData(position, chunk->GetBlockData());

    // 准备好周围的区块数据

    std::vector<Chunk> neighboringChunksStorage(8);
    neighboringChunksStorage[0].SetPosition({ position.x - 1, position.z - 1 });
    neighboringChunksStorage[1].SetPosition({ position.x - 1, position.z });
    neighboringChunksStorage[2].SetPosition({ position.x - 1, position.z + 1 });
    neighboringChunksStorage[3].SetPosition({ position.x, position.z - 1 });
    neighboringChunksStorage[4].SetPosition({ position.x, position.z + 1 });
    neighboringChunksStorage[5].SetPosition({ position.x + 1, position.z - 1 });
    neighboringChunksStorage[6].SetPosition({ position.x + 1, position.z });
    neighboringChunksStorage[7].SetPosition({ position.x + 1, position.z + 1 });

    for(auto &neighboringChunk : neighboringChunksStorage)
        LoadChunkBlockData(neighboringChunk.GetPosition(), &neighboringChunk.GetBlockData());

    // 计算光照

    Chunk *neighboringChunks[3][3] =
    {
        { &neighboringChunksStorage[0], &neighboringChunksStorage[1], &neighboringChunksStorage[2] },
        { &neighboringChunksStorage[3], chunk.get(), &neighboringChunksStorage[4] },
        { &neighboringChunksStorage[5], &neighboringChunksStorage[6], &neighboringChunksStorage[7] }
    };
    PropagateLightForCentreChunk(neighboringChunks);

    // 生成渲染模型

    const Chunk *constNeighboringChunks[3][3] =
    {
        { &neighboringChunksStorage[0], &neighboringChunksStorage[1], &neighboringChunksStorage[2] },
        { &neighboringChunksStorage[3], chunk.get(), &neighboringChunksStorage[4] },
        { &neighboringChunksStorage[5], &neighboringChunksStorage[6], &neighboringChunksStorage[7] }
    };

    for(int sx = 0; sx < CHUNK_SECTION_COUNT_X; ++sx)
    {
        for(int sz = 0; sz < CHUNK_SECTION_COUNT_Z; ++sz)
        {
            for(int sy = 0; sy < CHUNK_SECTION_COUNT_Y; ++sy)
            {
                chunk->RegenerateSectionModel({ sx, sy, sz }, constNeighboringChunks);
            }
        }
    }

    return chunk;
}

void ChunkLoader::SetChunkBlockDataInPool(int blockX, int blockY, int blockZ, BlockID id)
{
    blockDataPool_->ModifyBlockIDInPool({ blockX, blockY, blockZ }, id);
}

void ChunkLoader::LoadChunkBlockData(const ChunkPosition &position, ChunkBlockData *blockData)
{
    if(!blockDataPool_->GetChunkBlockData(position, blockData))
    {
        landGenerator_->Generate(position, blockData);
        auto blockDataInPool = std::make_unique<ChunkBlockData>(*blockData);
        blockDataPool_->AddChunkBlockData(position, std::move(blockDataInPool));
    }
}

void ChunkLoader::WorkerFunc(ChunkLoader *chunkLoader, PerThreadData *threadLocalData)
{
    assert(threadLocalData);
    for(;;)
    {
        auto newTask = threadLocalData->taskQueue.GetTask();
        if(newTask.is<ChunkLoaderTask_Stop>())
            break;
        chunkLoader->ExecuteTask(std::move(newTask));
    }
}

void ChunkLoader::ExecuteTask(ChunkLoaderTask &&task)
{
    if(auto load = task.as_if<ChunkLoaderTask_Load>())
    {
        if(skipLoading_)
            return;

        if(!load->chunk)
            load->chunk = LoadChunk(load->position);
        AddLoadingResult(std::move(load->chunk));

        log_->trace("load chunk({}, {})", load->position.x, load->position.z);
        return;
    }

    auto &unload = task.as<ChunkLoaderTask_Unload>();
    unload.chunk.reset();
}

VRPG_WORLD_END
