## Main Threads

**Rendering Thread**

```
do
    peek window messages
    send user input to logical thread (user input queue)
    update rendering camera
    for eack task in rendering queue
        render
    present
```

**Logic Thread**

```
do
    fetch user input from rendering thread (user input queue)
    update 3C
    send streaming tasks to steaming worker thread (streaming task queue)
    fetch streaming results from streaming worker thread (streaming result queue)
    rebuild rendering queue
```

**Streaming Thread**

```
do
    fetch new streaming task from worker task queue (streaming task queue)
    execute streaming task
    send streaming result to logical thread (streaming result queue)
```

## BlockInfoManagement

```

```

## Chunk Management

```
class BlockEffect;

class ChunkRenderingData
{
	virtual BlockEffect *GetBlockEffect();
	virtual void Render();
};

class Chunk
{
	vector<unique_ptr<ChunkRenderingData>> GenerateRenderingData();
	bool NeedToRegenerateRenderingData();
};

class ChunkManager
{
	void SetChunkCentre(int ckX, int ckY, int ckZ);
};
```
