#include <deque>
#include <queue>

#include <VRPG/Game/Chunk/ChunkLightPropagation.h>

VRPG_GAME_BEGIN

namespace
{
    bool OutOfBound(int x, int y, int z) noexcept
    {
        return x < 0 || x > 3 * CHUNK_SIZE_X - 1 ||
               y < 0 || y > CHUNK_SIZE_Y - 1 ||
               z < 0 || z > 3 * CHUNK_SIZE_Z - 1;
    }

    BlockBrightness GetLight(Chunk *(&chunks)[3][3], int x, int y, int z) noexcept
    {
        if(OutOfBound(x, y, z))
        {
            return BlockBrightness{ 0, 0, 0, 0 };
        }
        int chunkIndexX = x / CHUNK_SIZE_X, chunkIndexZ = z / CHUNK_SIZE_Z;
        int blockIndexX = x % CHUNK_SIZE_X, blockIndexZ = z % CHUNK_SIZE_Z;
        return chunks[chunkIndexX][chunkIndexZ]->GetBrightness({ blockIndexX, y, blockIndexZ });
    }

    void SetLight(Chunk *(&chunks)[3][3], int x, int y, int z, BlockBrightness brightness) noexcept
    {
        if(OutOfBound(x, y, z))
        {
            return;
        }
        int chunkIndexX = x / CHUNK_SIZE_X, chunkIndexZ = z / CHUNK_SIZE_Z;
        int blockIndexX = x % CHUNK_SIZE_X, blockIndexZ = z % CHUNK_SIZE_Z;
        chunks[chunkIndexX][chunkIndexZ]->SetBrightness({ blockIndexX, y, blockIndexZ }, brightness);
    }

    int GetHeight(Chunk *(&chunks)[3][3], int x, int z) noexcept
    {
        if(OutOfBound(x, 0, z))
        {
            return 0;
        }
        int chunkIndexX = x / CHUNK_SIZE_X, chunkIndexZ = z / CHUNK_SIZE_Z;
        int blockIndexX = x % CHUNK_SIZE_X, blockIndexZ = z % CHUNK_SIZE_Z;
        return chunks[chunkIndexX][chunkIndexZ]->GetHeight(blockIndexX, blockIndexZ);
    }

    BlockID GetID(Chunk *(&chunks)[3][3], int x, int y, int z) noexcept
    {
        if(OutOfBound(x, y, z))
        {
            return BLOCK_ID_VOID;
        }
        int chunkIndexX = x / CHUNK_SIZE_X, chunkIndexZ = z / CHUNK_SIZE_Z;
        int blockIndexX = x % CHUNK_SIZE_X, blockIndexZ = z % CHUNK_SIZE_Z;
        return chunks[chunkIndexX][chunkIndexZ]->GetID({ blockIndexX, y, blockIndexZ });
    }
}

void PropagateLightForCentreChunk(Chunk *(&chunks)[3][3])
{
    // IMPROVE: 这里面有大量边界检查和下标计算都是冗余的

    auto &blockDescMgr = BlockDescManager::GetInstance();
    std::queue<Vec3i> propagationQueue;

    // 若一个位置在区块范围内，就将他加入propagationQueue
    auto addToQueue = [&](int x, int y, int z)
    {
        if(!OutOfBound(x, y, z))
        {
            propagationQueue.push({ x, y, z });
        }
    };

    // 将一个位置的六个相邻位置加入光照更新队列中
    auto addNeighborToQueue = [&](int x, int y, int z)
    {
        addToQueue(x - 1, y, z);
        addToQueue(x + 1, y, z);
        addToQueue(x, y - 1, z);
        addToQueue(x, y + 1, z);
        addToQueue(x, y, z - 1);
        addToQueue(x, y, z + 1);
    };

    // 根据旁边方块的亮度、天光以及自发光更新某方块的亮度
    // 若更新成功，将它周围的方块加入传播队列中
    auto updateBlockBrightness = [&](const Vec3i &pos)
    {
        assert(!OutOfBound(pos.x, pos.y, pos.z));
        BlockID id = GetID(chunks, pos.x, pos.y, pos.z);
        auto desc = blockDescMgr.GetBlockDescription(id);
        BlockBrightness original = GetLight(chunks, pos.x, pos.y, pos.z);

        BlockBrightness posX = GetLight(chunks, pos.x + 1, pos.y, pos.z);
        BlockBrightness posY = GetLight(chunks, pos.x, pos.y + 1, pos.z);
        BlockBrightness posZ = GetLight(chunks, pos.x, pos.y, pos.z + 1);
        BlockBrightness negX = GetLight(chunks, pos.x - 1, pos.y, pos.z);
        BlockBrightness negY = GetLight(chunks, pos.x, pos.y - 1, pos.z);
        BlockBrightness negZ = GetLight(chunks, pos.x, pos.y, pos.z - 1);
        BlockBrightness maxNeighborLight = Max(
            Max(posX, Max(posY, posZ)),
            Max(negX, Max(negY, negZ)));

        BlockBrightness directSkyLight;
        int height = GetHeight(chunks, pos.x, pos.z);
        if(pos.y > height)
        {
            directSkyLight = BLOCK_BRIGHTNESS_SKY;
        }

        BlockBrightness emission = desc->InitialBrightness();
        BlockBrightness attenuation = desc->LightAttenuation();

        BlockBrightness propagated = Max(emission, Max(maxNeighborLight - attenuation, directSkyLight));

        if(propagated != original)
        {
            SetLight(chunks, pos.x, pos.y, pos.z, propagated);
            addNeighborToQueue(pos.x, pos.y, pos.z);
        }
    };

    // 填充光源亮度

    for(int x = 0; x < 3 * CHUNK_SIZE_X; ++x)
    {
        for(int z = 0; z < 3 * CHUNK_SIZE_Z; ++z)
        {
            int height = GetHeight(chunks, x, z);

            int pxHeight = GetHeight(chunks, x + 1, z);
            int nxHeight = GetHeight(chunks, x - 1, z);
            int pzHeight = GetHeight(chunks, x, z + 1);
            int nzHeight = GetHeight(chunks, x, z - 1);
            int maxNeighborHeight = (std::max)(
                (std::max)(pxHeight, nxHeight),
                (std::max)(pzHeight, nzHeight));

            for(int y = 0; y <= height; ++y)
            {
                BlockID id = GetID(chunks, x, y, z);
                auto desc = blockDescMgr.GetBlockDescription(id);
                if(desc->IsLightSource())
                {
                    SetLight(chunks, x, y, z, desc->InitialBrightness());
                    addNeighborToQueue(x, y, z);
                }
            }

            for(int y = height + 1; y < CHUNK_SIZE_Y; ++y)
            {
                SetLight(chunks, x, y, z, BLOCK_BRIGHTNESS_SKY);
            }

            int ySourceEnd = maxNeighborHeight + 1;
            for(int y = height + 1; y <= ySourceEnd; ++y)
            {
                addNeighborToQueue(x, y, z);
            }
        }
    }

    // 亮度传播

    while(!propagationQueue.empty())
    {
        Vec3i pos = propagationQueue.front();
        propagationQueue.pop();
        updateBlockBrightness(pos);
    }
}

VRPG_GAME_END
