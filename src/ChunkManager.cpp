#include <imgui/imgui.h>
#include <ChunkManager.hpp>
#include <algorithm>

static void builderThreadRoutine(ChunkManager* manager, glm::i32vec2 firstPosition, int index) {
	std::unique_lock<std::mutex> ulock(manager->ucMutex, std::defer_lock);

	while (true) {
		ulock.lock();
		manager->updateCondition.wait(ulock);
		ulock.unlock();
		if (!manager->ThreadShouldRun)
			break;
		while (true) {
			if (!manager->ThreadShouldRun)
				break;
			manager->queueInMutex.lock();
			if (manager->buildingQueueIn.empty()) {
				manager->queueInMutex.unlock();
				break;
			}
			auto it = manager->buildingQueueIn.begin();
			auto task = *it;
			manager->buildingQueueIn.erase(it);
			manager->queueInMutex.unlock();

			Chunk* c = new Chunk();
			chunkCreateGeometry(c, task, MOUNTAIN, &(manager->heightMap));
			manager->queueOutMutex.lock();
			manager->buildingQueueOut.push_back(c);
			manager->queueOutMutex.unlock();
		}
	}
}

glm::i32vec2 ChunkManager::GetChunkPosition(glm::vec3 pos) {
	glm::i32vec2 chunkPosition((s32)pos.x / CHUNK_SIZE,
                       (s32)pos.z / CHUNK_SIZE);

	if (pos.x < 0)
		chunkPosition.x--;
	if (pos.z < 0)
		chunkPosition.y--;

	return chunkPosition;
}

ChunkManager::ChunkManager(glm::vec3 playerPos, Frustrum* frustrum) : frustrum(frustrum) {
	ThreadShouldRun = true;
	playerPos.y = 0.0f;
	auto chunkPosition = GetChunkPosition(playerPos);
	lastPosition = chunkPosition;

	glm::i32vec2 cursor(0, 0);

	for (cursor.x = chunkPosition.x - chunkLoadRadius;
			cursor.x <= chunkPosition.x + chunkLoadRadius;
			cursor.x++) {
		for (cursor.y = chunkPosition.y - chunkLoadRadius;
				cursor.y <= chunkPosition.y + chunkLoadRadius;
				cursor.y++) {
			Chunk* c = new Chunk();
			chunkCreateGeometry(c, cursor, MOUNTAIN, &heightMap);
			chunks.push_back(c);
		}
	}

	for (int i = 0; i < NUM_MANAGER_THREADS; i++)
		builderRoutineThreads[i] = std::thread(builderThreadRoutine, this, chunkPosition, i);
}

ChunkManager::~ChunkManager() {
	ThreadShouldRun = false;
	ucMutex.lock();
	updateCondition.notify_all();
	ucMutex.unlock();
	for (int i = 0; i < NUM_MANAGER_THREADS; i++)
		builderRoutineThreads[i].join();
	for (auto& c : chunks)
	{
		chunkDestroy(c);
		delete c;
	}
}

bool ChunkManager::ChunkIsLoaded(glm::i32vec2 pos) {
	for (auto& it : chunks) {
		if (it->position.x == pos.x && it->position.y == pos.y)
			return true;
	}
	return false;
}

void ChunkManager::Update(glm::vec3 playerPos) {
	position = GetChunkPosition(playerPos);


	ImGui::Text("Chunk Position: %d %d\n", position.x, position.y);
	ImGui::Text("Chunks loaded: %lu\n", chunks.size());

	ImGui::SliderInt("Chunk load radius", &chunkLoadRadius, 1, 32);
	ImGui::SliderInt("Chunk unload radius", &chunkUnloadRadius, 1, 32);
	ImGui::Text("Render distance: %d cubes\n", chunkLoadRadius * CHUNK_SIZE);
	
	// Flush building queue
	queueOutMutex.lock();
	while (!buildingQueueOut.empty()) {
		auto elem = buildingQueueOut.back();
		buildingQueueOut.pop_back();
		if (!ChunkIsLoaded(elem->position))
			chunks.push_back(elem);
		else {
			// Race condition: the element was built twice
			chunkDestroy(elem);
			delete elem;
		}
	}
	queueOutMutex.unlock();

	if (position == lastPosition)
		return ;

	std::vector<glm::i32vec2> chunksToBuild;
	s32 deltaX = position.x - lastPosition.x;
	s32 deltaY = position.y - lastPosition.y;

	if (deltaX != 0 || deltaY != 0) {
		auto it = std::begin(chunks);
		while (it != std::end(chunks)) {
			auto cpos = (*it)->position;
			if (abs(position.x - cpos.x) > chunkUnloadRadius || abs(position.y - cpos.y) > chunkUnloadRadius) {
				chunkDestroy(*it);
				delete *it;
				it = chunks.erase(it);
			} else {
				++it;
			}
		}
	}
	if (deltaX < 0) {
		for (s32 x = position.x - chunkLoadRadius; x < lastPosition.x - chunkLoadRadius; x++) {
			for (s32 y = position.y - chunkLoadRadius; y <= position.y + chunkLoadRadius; y++) {
				chunksToBuild.push_back(glm::i32vec2(x, y));
			}
		}
	}
	if (deltaX > 0) {
		for (s32 x = position.x + chunkLoadRadius; x > lastPosition.x + chunkLoadRadius; x--) {
			for (s32 y = position.y - chunkLoadRadius; y <= position.y + chunkLoadRadius; y++) {
				chunksToBuild.push_back(glm::i32vec2(x, y));
			}
		}
	}
	if (deltaY < 0) {
		for (s32 y = position.y - chunkLoadRadius; y < lastPosition.y - chunkLoadRadius; y++) {
			for (s32 x = position.x - chunkLoadRadius; x <= position.x + chunkLoadRadius; x++) {
				chunksToBuild.push_back(glm::i32vec2(x, y));
			}
		}
	}
	if (deltaY > 0) {
		for (s32 y = position.y + chunkLoadRadius; y > lastPosition.y + chunkLoadRadius; y--) {
			for (s32 x = position.x - chunkLoadRadius; x <= position.x + chunkLoadRadius; x++) {
				chunksToBuild.push_back(glm::i32vec2(x, y));
			}
		}
	}

	if (chunksToBuild.size() > 0) {
		queueInMutex.lock();

		// Flush obsolete orders
		auto it = std::begin(buildingQueueIn);
		while (it != std::end(buildingQueueIn)) {
			auto cpos = *it;
			if (abs(position.x - cpos.x) > chunkUnloadRadius || abs(position.y - cpos.y) > chunkUnloadRadius) {
				it = buildingQueueIn.erase(it);
			
			} else {
				++it;
			}
		}

		for (auto& elem : chunksToBuild)
			buildingQueueIn.push_back(elem);
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

	for (auto& c : chunks) {
		bounds.min = c->worldPosition;
		bounds.max = glm::vec3(c->worldPosition) + sizeOffset;
		// Check if any of eight corners of the chunk is in sight
		if (frustrum->IsCubeIn(bounds)) {
			chunkDraw(c, s);
			skipped++;
		}

	}
	ImGui::Text("Chunks drawn: %u / %lu\n", skipped, chunks.size());
}
