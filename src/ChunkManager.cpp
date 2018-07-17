#include <imgui/imgui.h>
#include <ChunkManager.hpp>
#include <algorithm>

static void builderThreadRoutine(ChunkManager* manager, glm::i32vec2 firstPosition) {
	std::unique_lock<std::mutex> ulock(manager->ucMutex);

	while (true) {
		manager->updateCondition.wait(ulock);
		if (!manager->ThreadShouldRun)
			break;
		while (true) {
			manager->queueInMutex.lock();
			if (manager->buildingQueueIn.empty()) {
				manager->queueInMutex.unlock();
				break;
			}
			glm::i32vec2 task = manager->buildingQueueIn.back();
			manager->buildingQueueIn.pop_back();
			manager->queueInMutex.unlock();

			Chunk* c = new Chunk(MOUNTAIN, task, &(manager->heightMap));
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
			chunks.push_back(new Chunk(MOUNTAIN, cursor, &heightMap));
		}
	}

	builderRoutineThread = std::thread(builderThreadRoutine, this, chunkPosition);
}

ChunkManager::~ChunkManager() {
	ThreadShouldRun = false;
	queueOutMutex.unlock();
	updateCondition.notify_all();
	builderRoutineThread.join();
	for (auto& c : chunks)
		delete c;
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
	
	// Flush building queue
	queueOutMutex.lock();
	while (!buildingQueueOut.empty()) {
		auto elem = buildingQueueOut.back();
		buildingQueueOut.pop_back();
		if (!ChunkIsLoaded(elem->position))
			chunks.push_back(elem);
		else {
			// Race condition: the element was built twice
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
		for (auto& elem : chunksToBuild)
			buildingQueueIn.push_back(elem);
		queueInMutex.unlock();
		updateCondition.notify_one();
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
			c->Draw(s);
			skipped++;
		}

	}
	ImGui::Text("Chunks drawn: %u / %lu\n", skipped, chunks.size());
}
