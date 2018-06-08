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

			Chunk* c = new Chunk(args.biome, args.pos, manager->GetHeightMap());
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

glm::vec3 ChunkManager::GetChunkPosition(glm::vec3 pos) {
	glm::vec3 chunkPosition((int)(pos.x / CHUNK_SIZE) * CHUNK_SIZE,
	                        (int)(pos.y / CHUNK_SIZE) * CHUNK_SIZE,
	                        (int)(pos.z / CHUNK_SIZE) * CHUNK_SIZE);

	if (pos.x < 0)
		chunkPosition.x -= CHUNK_SIZE;
	if (pos.y < 0)
		chunkPosition.y -= CHUNK_SIZE;
	if (pos.z < 0)
		chunkPosition.z -= CHUNK_SIZE;

	return chunkPosition;
}

ChunkManager::ChunkManager(glm::vec3 playerPos, Frustrum* frustrum)
    : frustrum(frustrum) {
	playerPos.y = 0.0f;
	auto chunkPosition = GetChunkPosition(playerPos);

	chunks.push_back(new Chunk(eBiome::FOREST, chunkPosition, &(heightMap)));
	builderThread = std::thread(fnBuilderThread, this);
}

bool ChunkManager::ChunkIsLoaded(glm::vec3 pos) {
	s32 ax = (s32)pos.x;
	s32 ay = (s32)pos.y;
	s32 az = (s32)pos.z;
	for (auto& it : chunks) {
		s32 bx = (s32)it->GetPosition().x;
		s32 by = (s32)it->GetPosition().y;
		s32 bz = (s32)it->GetPosition().z;
		if (ax == bx && ay == by && az == bz)
			return true;
	}
	return false;
}

bool ChunkManager::ChunkIsLoaded(s32 ax, s32 ay, s32 az) {
	for (auto& it : chunks) {
		s32 bx = (s32)it->GetPosition().x;
		s32 by = (s32)it->GetPosition().y;
		s32 bz = (s32)it->GetPosition().z;
		if (ax == bx && ay == by && az == bz)
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

bool ChunkManager::ShouldLoadChunk(glm::vec3 playerPos, glm::vec3 position) {
	s32 distance = glm::distance(playerPos, position);

	if (distance < chunkLoadRadius * CHUNK_SIZE)
		return true;
	return false;
}

bool ChunkManager::ShouldUnloadChunk(glm::vec3 playerPos, glm::vec3 position) {
	s32 distance = glm::distance(playerPos, position);

	if (distance < chunkUnloadRadius * CHUNK_SIZE)
		return false;
	return true;
}

void ChunkManager::Update(glm::vec3 playerPos) {
	playerPos.y = 0.0f;
	auto chunkPosition = GetChunkPosition(playerPos);

	queueOutMutex.lock();
	while (!BuildingQueueOut.empty()) {
		auto elem = BuildingQueueOut.back();
		BuildingQueueOut.pop_back();
		auto position = elem->GetPosition();
		if (!ChunkIsLoaded(elem->GetPosition()))
			chunks.push_back(elem);
		else {
			// Race condition: the element was built twice
			delete elem;
		}
	}
	queueOutMutex.unlock();


	auto it = std::begin(chunks);
	while (it != std::end(chunks)) {
		if (ShouldUnloadChunk(playerPos, (*it)->GetPosition())) {
			delete *it;
			it = chunks.erase(it);
		} else {
			++it;
		}
	}

	bool buildNeeded = false;

	queueInMutex.lock();
	glm::vec3 vecLoadRadius =
	    glm::vec3(chunkLoadRadius, chunkLoadRadius, chunkLoadRadius) *
	    (float)CHUNK_SIZE;

	s32 y = chunkPosition.y - vecLoadRadius.y;

	s32 maxX = chunkPosition.x + vecLoadRadius.x;
	s32 maxY = chunkPosition.y + vecLoadRadius.y;
	s32 maxZ = chunkPosition.z + vecLoadRadius.z;

	// TODO: vertical build
	y = 0;

	for (s32 x = chunkPosition.x - vecLoadRadius.x; x < maxX; x += CHUNK_SIZE) {
		for (s32 z = chunkPosition.z - vecLoadRadius.z; z < maxZ;
		     z += CHUNK_SIZE) {
			if (!ShouldLoadChunk(playerPos, glm::vec3(x, y, z)))
				continue;
			if (ChunkIsLoaded(x, y, z))
				continue;

			bool isBeingBuilt = false;
			for (auto args : BuildingQueueIn) {
				if ((s32)args.pos.x == x && (s32)args.pos.y == y &&
				    (s32)args.pos.z == z) {
					isBeingBuilt = true;
					break;
				}
			}

			if (!isBeingBuilt) {
				buildNeeded = true;
				chunkArguments args = {eBiome::MOUNTAIN, glm::vec3(x, y, z)};
				BuildingQueueIn.push_back(args);
			}
		}
	}
	queueInMutex.unlock();
	if (buildNeeded)
		buildCondition.notify_one();


	ImGui::Text("Chunk Position: %f %f %f\n", chunkPosition.x, chunkPosition.y,
	            chunkPosition.z);
	ImGui::Text("Chunks loaded: %lu\n", chunks.size());

	ImGui::SliderInt("Chunk load radius", &chunkLoadRadius, 1, 20);
	ImGui::SliderInt("Chunk unload radius", &chunkUnloadRadius, 1, 20);
}

void ChunkManager::Draw(Shader s) {
	const glm::vec3 xOffset((float)CHUNK_SIZE, 0.0f, 0.0f);
	const glm::vec3 yOffset(0.0f, (float)CHUNK_SIZE, 0.0f);
	const glm::vec3 zOffset(0.0f, 0.0f, (float)CHUNK_SIZE);

	for (auto& c : chunks) {
		// Check if any of eight corners of the chunk is in sight
		if (frustrum->IsPointIn(c->GetPosition()) ||
		    frustrum->IsPointIn(c->GetPosition() + xOffset) ||
		    frustrum->IsPointIn(c->GetPosition() + yOffset) ||
		    frustrum->IsPointIn(c->GetPosition() + zOffset) ||
		    frustrum->IsPointIn(c->GetPosition() + xOffset + yOffset) ||
		    frustrum->IsPointIn(c->GetPosition() + zOffset + yOffset) ||
		    frustrum->IsPointIn(c->GetPosition() + xOffset + zOffset) ||
		    frustrum->IsPointIn(c->GetPosition() + xOffset + yOffset +
		                        zOffset)) {
			c->Draw(s);
		}
	}
}
