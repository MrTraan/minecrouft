#include <imgui/imgui.h>
#include <ChunkManager.hpp>
#include <algorithm>

static void fnBuilderThread(ChunkManager* manager) {
	std::unique_lock<std::mutex> ulock(manager->builderMutex);

	while (manager->ThreadShouldRun) {
		manager->buildCondition.wait(ulock);

		while (!manager->BuildingQueueIn.empty()) {
			manager->queueInMutex.lock();
			auto it = manager->BuildingQueueIn.begin();
			auto args = *it;
			manager->queueInMutex.unlock();

			Chunk* c = new Chunk(args);
			manager->queueOutMutex.lock();
			manager->BuildingQueueOut.push_back(c);
			manager->queueOutMutex.unlock();

			manager->queueInMutex.lock();
			it = std::find_if(manager->BuildingQueueIn.begin(), manager->BuildingQueueIn.end(),
				[&args](const chunkArguments& it) { return it.pos == args.pos; });
			if (it != manager->BuildingQueueIn.end())
				manager->BuildingQueueIn.erase(it);
			manager->queueInMutex.unlock();
		}
	}
}

glm::i32vec2 ChunkManager::GetChunkPosition(glm::vec3 pos) {
	glm::i32vec2 chunkPosition((s32)(pos.x / CHUNK_SIZE),
	                        (s32)(pos.z / CHUNK_SIZE));

	if (pos.x < 0)
		chunkPosition.x--;
	if (pos.y < 0)
		chunkPosition.y--;

	return chunkPosition;
}

ChunkManager::ChunkManager(glm::vec3 playerPos, Frustrum* frustrum)
    : frustrum(frustrum) {
	instance = this;
	playerPos.y = 0.0f;
	auto chunkPosition = GetChunkPosition(playerPos);

	chunkArguments args;

	args.biome = eBiome::FOREST;
	args.pos = chunkPosition;

	chunks.push_back(new Chunk(args));
	builderThread = std::thread(fnBuilderThread, this);
}

bool ChunkManager::ChunkIsLoaded(glm::i32vec2 pos) {
	for (auto& it : chunks) {
		if (it->position.x == pos.x && it->position.y == pos.y)
			return true;
	}
	return false;
}

bool ChunkManager::ChunkIsLoaded(s32 ax, s32 ay) {
	for (auto& it : chunks) {
		if (it->position.x == ax && it->position.y == ay)
			return true;
	}
	return false;
}

ChunkManager::~ChunkManager() {
	ThreadShouldRun = false;
	buildCondition.notify_all();
	builderThread.join();
	for (auto& c : chunks)
		delete c;
}

bool ChunkManager::ShouldLoadChunk(glm::i32vec2 currentPos, glm::i32vec2 position) {
	glm::i32vec2 ray(position - currentPos);
	float distance = sqrtf(ray.x * ray.x + ray.y * ray.y);

	if (distance < chunkLoadRadius)
		return true;
	return false;
}

bool ChunkManager::ShouldUnloadChunk(glm::i32vec2 currentPos, glm::i32vec2 position) {
	glm::i32vec2 ray(position - currentPos);
	float distance = sqrtf(ray.x * ray.x + ray.y * ray.y);

	if (distance < chunkUnloadRadius)
		return false;
	return true;
}

void ChunkManager::Update(glm::vec3 playerPos) {
	static glm::i32vec2 lastPos;
	static bool firstFrame = true;

	auto chunkPosition = GetChunkPosition(playerPos);
	
	ImGui::Text("Chunk Position: %d %d\n", chunkPosition.x, chunkPosition.y);
	ImGui::Text("Chunks loaded: %lu\n", chunks.size());

	ImGui::SliderInt("Chunk load radius", &chunkLoadRadius, 1, 32);
	ImGui::SliderInt("Chunk unload radius", &chunkUnloadRadius, 1, 32);

	queueOutMutex.lock();
	while (!BuildingQueueOut.empty()) {
		auto elem = BuildingQueueOut.back();
		BuildingQueueOut.pop_back();
		if (!ChunkIsLoaded(elem->position))
			chunks.push_back(elem);
		else {
			// Race condition: the element was built twice
			delete elem;
		}
	}
	queueOutMutex.unlock();


	auto it = std::begin(chunks);
	while (it != std::end(chunks)) {
		if (ShouldUnloadChunk(chunkPosition, (*it)->position)) {
			delete *it;
			it = chunks.erase(it);
		} else {
			++it;
		}
	}

	if (!firstFrame && chunkPosition == lastPos)
		return;

	bool buildNeeded = false;

	queueInMutex.lock();

	glm::ivec2 cursor(0, 0);

	for (cursor.x = chunkPosition.x - chunkLoadRadius; cursor.x < chunkPosition.x + chunkLoadRadius; cursor.x++) {
		for (cursor.y = chunkPosition.y - chunkLoadRadius; cursor.y < chunkPosition.y + chunkLoadRadius; cursor.y++) {
			if (ChunkIsLoaded(cursor))
				continue;
			if (!ShouldLoadChunk(chunkPosition, cursor))
				continue;

			bool isBeingBuilt = false;
			for (auto& args : BuildingQueueIn) {
				if (args.pos == cursor) {
					isBeingBuilt = true;
					break;
				}
			}

			if (!isBeingBuilt) {
				buildNeeded = true;
				chunkArguments args;
				args.biome = MOUNTAIN;
				args.pos = cursor;
				args.leftNeighbor = GetNeighbor(cursor, eDirection::LEFT);
				args.rightNeighbor = GetNeighbor(cursor, eDirection::RIGHT);
				args.frontNeighbor = GetNeighbor(cursor, eDirection::FRONT);
				args.backNeighbor = GetNeighbor(cursor, eDirection::BACK);
				BuildingQueueIn.push_back(args);
			}
		}
	}
	queueInMutex.unlock();
	if (buildNeeded)
		buildCondition.notify_one();

	lastPos = chunkPosition;
	firstFrame = false;
}

void ChunkManager::Draw(Shader s) {
	static glm::vec3 sizeOffset = glm::vec3((float)CHUNK_SIZE, (float)CHUNK_HEIGHT, (float)CHUNK_SIZE);
	Aabb bounds;

	u32 skipped = 0;

	for (auto& c : chunks) {
		bounds.min = c->worldPosition;
		bounds.max = c->worldPosition + sizeOffset;
		// Check if any of eight corners of the chunk is in sight
		if (frustrum->IsCubeIn(bounds)) {
			c->Draw(s);
			skipped++;
		}
	
	}
	ImGui::Text("Chunks drawn: %lu / %lu\n", skipped, chunks.size());
}

Chunk* ChunkManager::GetNeighbor(glm::i32vec2 pos, eDirection direction) {
	if (chunks.size() == 0)
		return nullptr;

	if (direction == eDirection::LEFT) {
			auto it = std::find_if(chunks.begin(), chunks.end(),
				[&pos](auto& it) { return it->position.x == pos.x - 1 && it->position.y == pos.y; });
			if (it != chunks.end())
				return *it;
			return nullptr;
	}

	if (direction == eDirection::RIGHT) {
			auto it = std::find_if(chunks.begin(), chunks.end(),
				[&pos](auto& it) { return it->position.x == pos.x + 1 && it->position.y == pos.y; });
			if (it != chunks.end())
				return *it;
			return nullptr;
	}
	
	if (direction == eDirection::FRONT) {
			auto it = std::find_if(chunks.begin(), chunks.end(),
				[&pos](auto& it) { return it->position.x == pos.x && it->position.y == pos.y - 1; });
			if (it != chunks.end())
				return *it;
			return nullptr;
	}
	
	if (direction == eDirection::BACK) {
			auto it = std::find_if(chunks.begin(), chunks.end(),
				[&pos](auto& it) { return it->position.x == pos.x && it->position.y == pos.y + 1; });
			if (it != chunks.end())
				return *it;
			return nullptr;
	}

	return nullptr;
}

ChunkManager* ChunkManager::instance = nullptr;
