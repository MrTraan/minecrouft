#include <imgui/imgui.h>
#include <ChunkManager.hpp>
#include <algorithm>

static void builderThreadRoutine(ChunkManager* manager, int index)
{
	std::unique_lock<std::mutex> ulock(manager->ucMutex, std::defer_lock);
	eBlockType mask[CHUNK_SIZE * CHUNK_HEIGHT];

	while (true)
	{
		ulock.lock();
		manager->updateCondition.wait(ulock);
		ulock.unlock();
		if (!manager->ThreadShouldRun)
			break;
		while (true)
		{
			if (!manager->ThreadShouldRun)
				goto EXIT_THREAD_ROUTINE;
			manager->queueInMutex.lock();
			if (manager->buildingQueueIn.empty())
			{
				manager->queueInMutex.unlock();
				break;
			}
			auto it = manager->buildingQueueIn.begin();
			auto task = *it;
			manager->buildingQueueIn.erase(it);
			manager->queueInMutex.unlock();

			chunkCreateGeometry(task, &(manager->heightMap), mask);
			manager->queueOutMutex.lock();
			manager->buildingQueueOut.push_back(task);
			manager->queueOutMutex.unlock();
		}
	}
EXIT_THREAD_ROUTINE:
	(void)0;
}

glm::i32vec2 ChunkManager::GetChunkPosition(glm::vec3 pos)
{
	glm::i32vec2 chunkPosition((s32)pos.x / CHUNK_SIZE,
                       (s32)pos.z / CHUNK_SIZE);

	if (pos.x < 0)
		chunkPosition.x--;
	if (pos.z < 0)
		chunkPosition.y--;

	return chunkPosition;
}

ChunkManager::ChunkManager(glm::vec3 playerPos, Frustrum* frustrum) : frustrum(frustrum)
{
	ThreadShouldRun = true;
	playerPos.y = 0.0f;
	auto chunkPosition = GetChunkPosition(playerPos);
	lastPosition = chunkPosition;
	eBlockType mask[CHUNK_SIZE * CHUNK_HEIGHT];

	glm::u16vec2 cursor(0, 0);

	for (cursor.x = chunkPosition.x - chunkLoadRadius;
			cursor.x <= chunkPosition.x + chunkLoadRadius;
			cursor.x++) {
		for (cursor.y = chunkPosition.y - chunkLoadRadius;
				cursor.y <= chunkPosition.y + chunkLoadRadius;
				cursor.y++)
		{
			ChunkCoordinates pos = createChunkCoordinates(cursor.x, cursor.y);
			auto chunk = popChunkFromPool();
			chunk->position = pos;
			chunk->worldPosition = glm::i32vec3((s32)getXCoord(pos) * CHUNK_SIZE, 0, (s32)getZCoord(pos) * CHUNK_SIZE);
			chunk->biome = eBiome::MOUNTAIN;
			chunkCreateGeometry(chunk, &heightMap, mask);
			chunks[pos] = chunk;
		}
	}

	for (int i = 0; i < NUM_MANAGER_THREADS; i++)
		builderRoutineThreads[i] = std::thread(builderThreadRoutine, this, i);
}

ChunkManager::~ChunkManager() {
	ThreadShouldRun = false;
	ucMutex.lock();
	updateCondition.notify_all();
	ucMutex.unlock();
	for (int i = 0; i < NUM_MANAGER_THREADS; i++)
		builderRoutineThreads[i].join();
	for (auto& e : chunks)
	{
		meshDeleteBuffers(e.second->mesh);
		chunkDestroy(e.second);
		delete e.second;
	}
	while (poolHead != nullptr)
	{
		chunkDestroy(poolHead);
		poolHead = poolHead->poolNextItem;
	}

}

inline bool ChunkManager::ChunkIsLoaded(ChunkCoordinates pos)
{
	return chunks.find(pos) != chunks.end();
}

inline bool ChunkManager::ChunkIsLoaded(u16 x, u16 y)
{
	return chunks.find(createChunkCoordinates(x, y)) != chunks.end();
}

void ChunkManager::Update(glm::vec3 playerPos) {
	position = GetChunkPosition(playerPos);
	std::vector<ChunkCoordinates> chunksToBuild;
	s32 deltaX = position.x - lastPosition.x;
	s32 deltaY = position.y - lastPosition.y;

	ImGui::Text("Chunk Position: %d %d\n", position.x, position.y);
	ImGui::Text("Chunks loaded: %lu\n", chunks.size());

	static int newChunkLoadRadius = chunkLoadRadius;
	static int newChunkUnloadRadius = chunkUnloadRadius;
	ImGui::SliderInt("Chunk load radius", &newChunkLoadRadius, 1, 32);
	ImGui::SliderInt("Chunk unload radius", &newChunkUnloadRadius, 1, 32);

	bool forceUpdate = false;
	if (ImGui::Button("Apply"))
	{
		for (u16 x = position.x - newChunkLoadRadius;
			x <= position.x + newChunkLoadRadius;
			x++)
		{
			for (u16 y = position.y - newChunkLoadRadius;
				y <= position.y + newChunkLoadRadius;
				y++)
			{
				if (!ChunkIsLoaded(x, y))
					chunksToBuild.push_back(createChunkCoordinates(x, y));
			}
		}
		chunkLoadRadius = newChunkLoadRadius;
		chunkUnloadRadius = newChunkUnloadRadius;
		forceUpdate = true;
	}
	ImGui::Text("Render distance: %d cubes\n", chunkLoadRadius * CHUNK_SIZE);
	
	// Flush building queue
	queueOutMutex.lock();
	while (!buildingQueueOut.empty()) {
		auto elem = buildingQueueOut.back();
		buildingQueueOut.pop_back();
		if (!ChunkIsLoaded(elem->position))
			chunks[elem->position] = elem;
		else {
			// Race condition: the element was built twice
			printf("Race condition?\n");
			if (elem->mesh->isBound)
				meshDeleteBuffers(elem->mesh);
			pushChunkToPool(elem);
		}
	}
	queueOutMutex.unlock();

	if (position == lastPosition && !forceUpdate)
		return ;

	if (deltaX != 0 || deltaY != 0 || forceUpdate) {
		
		for (auto it = std::begin(chunks); it != std::end(chunks);)
		{
			auto cpos = it->second->position;
			if (abs(position.x - getXCoord(cpos)) > chunkUnloadRadius || abs(position.y - getZCoord(cpos)) > chunkUnloadRadius)
			{
				if (it->second->mesh->isBound)
					meshDeleteBuffers(it->second->mesh);
				pushChunkToPool(it->second);
				it = chunks.erase(it);
			}
			else
				++it;
		}
	}

	if (deltaX < 0)
	{
		for (u16 x = position.x - chunkLoadRadius; x < lastPosition.x - chunkLoadRadius; x++)
			for (u16 y = position.y - chunkLoadRadius; y <= position.y + chunkLoadRadius; y++)
				if (!ChunkIsLoaded(x, y))
					chunksToBuild.push_back(createChunkCoordinates(x, y));
	}
	
	if (deltaX > 0)
	{
		for (u16 x = position.x + chunkLoadRadius; x > lastPosition.x + chunkLoadRadius; x--)
			for (u16 y = position.y - chunkLoadRadius; y <= position.y + chunkLoadRadius; y++)
				if (!ChunkIsLoaded(x, y))
					chunksToBuild.push_back(createChunkCoordinates(x, y));
	}

	if (deltaY < 0)
	{
		for (u16 y = position.y - chunkLoadRadius; y < lastPosition.y - chunkLoadRadius; y++)
			for (u16 x = position.x - chunkLoadRadius; x <= position.x + chunkLoadRadius; x++)
				if (!ChunkIsLoaded(x, y))
					chunksToBuild.push_back(createChunkCoordinates(x, y));
	}

	if (deltaY > 0)
	{
		for (u16 y = position.y + chunkLoadRadius; y > lastPosition.y + chunkLoadRadius; y--)
			for (u16 x = position.x - chunkLoadRadius; x <= position.x + chunkLoadRadius; x++)
				if (!ChunkIsLoaded(x, y))
					chunksToBuild.push_back(createChunkCoordinates(x, y));
	}

	if (chunksToBuild.size() > 0)
	{
		queueInMutex.lock();

		// Flush obsolete orders
		for (auto it = std::begin(buildingQueueIn); it != std::end(buildingQueueIn);)
		{
			auto cpos = (*it)->position;
			if (abs(position.x - getXCoord(cpos)) > chunkUnloadRadius || abs(position.y - getZCoord(cpos)) > chunkUnloadRadius)
			{
				pushChunkToPool(*it);
				it = buildingQueueIn.erase(it);
			}
			else
				++it;
		}

		for (auto& pos : chunksToBuild)
		{
			auto chunk = popChunkFromPool();
			chunk->position = pos;
			chunk->worldPosition = glm::i32vec3((s32)getXCoord(pos) * CHUNK_SIZE, 0, (s32)getZCoord(pos) * CHUNK_SIZE);
			chunk->biome = eBiome::MOUNTAIN;
			buildingQueueIn.push_back(chunk);
		}

		queueInMutex.unlock();
		
		ucMutex.lock();
		updateCondition.notify_all();
		ucMutex.unlock();
	}
	lastPosition = position;
}

void ChunkManager::Draw(Shader s) {
	static glm::vec3 sizeOffset = glm::vec3((float)CHUNK_SIZE, (float)CHUNK_HEIGHT, (float)CHUNK_SIZE);
	Aabb bounds;

	u32 skipped = 0;

	for (auto& it : chunks)
	{
		bounds.min = it.second->worldPosition;
		bounds.max = glm::vec3(it.second->worldPosition) + sizeOffset;
		// Check if any of eight corners of the chunk is in sight
		if (frustrum->IsCubeIn(bounds))
		{
			chunkDraw(it.second, s);
			skipped++;
		}

	}
	ImGui::Text("Chunks drawn: %u / %lu\n", skipped, chunks.size());
}

Chunk* ChunkManager::popChunkFromPool()
{
	if (poolHead == nullptr)
	{
		return preallocateChunk();
	}
	auto item = poolHead;
	poolHead = poolHead->poolNextItem;
	item->poolNextItem = nullptr;
	return item;
}

void ChunkManager::pushChunkToPool(Chunk* item)
{
	if (poolHead == nullptr)
	{
		poolHead = item;
	}
	else
	{
		item->poolNextItem = poolHead;
		poolHead = item;
	}
}
