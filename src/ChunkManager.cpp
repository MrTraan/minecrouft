#include <imgui/imgui.h>
#include <ChunkManager.hpp>

static void fnBuilderThread(ChunkManager* manager) {
	std::unique_lock<std::mutex> ulock(manager->builderMutex);

	while (manager->ThreadShouldRun) {
		manager->GetBuildCondition()->wait(ulock);

		while (!manager->BuildingQueueIn.empty()) {
			manager->queueInMutex.lock();
			auto it = manager->BuildingQueueIn.end() - 1;
			auto args = *it;
			manager->queueInMutex.unlock();

			Chunk* c = new Chunk(args.biome, args.pos, manager->GetHeightMap());
			manager->queueOutMutex.lock();
			manager->BuildingQueueOut.push_back(c);
			manager->queueOutMutex.unlock();

			manager->queueInMutex.lock();
			manager->BuildingQueueIn.erase(it);
			manager->queueInMutex.unlock();
		}
	}
}

ChunkManager::ChunkManager(glm::vec3 playerPos, Frustrum* frustrum)
    : ThreadShouldRun(true) {
	this->frustrum = frustrum;
	glm::vec3 chunkPosition((int)(playerPos.x / CHUNK_SIZE) * CHUNK_SIZE, 0,
	                        (int)(playerPos.z / CHUNK_SIZE) * CHUNK_SIZE);

	if (playerPos.x < 0)
		chunkPosition.x -= CHUNK_SIZE;
	if (playerPos.z < 0)
		chunkPosition.z -= CHUNK_SIZE;

	this->PushChunk(chunkPosition, new Chunk(eBiome::FOREST, chunkPosition,
	                                         &(this->heightMap)));
	builderThread = std::thread(fnBuilderThread, this);
}

ChunkManager::~ChunkManager() {
	this->ThreadShouldRun = false;
	buildCondition.notify_all();
	builderThread.join();
	for (auto& c : chunks)
		delete c.second;
}

void ChunkManager::Update(glm::vec3 playerPos) {
	glm::vec3 chunkPosition((int)(playerPos.x / CHUNK_SIZE) * CHUNK_SIZE, 0,
	                        (int)(playerPos.z / CHUNK_SIZE) * CHUNK_SIZE);

	if (playerPos.x < 0)
		chunkPosition.x -= CHUNK_SIZE;
	if (playerPos.z < 0)
		chunkPosition.z -= CHUNK_SIZE;

	int chunkMaxDistance = 6;

	queueOutMutex.lock();
	while (!BuildingQueueOut.empty()) {
		auto elem = BuildingQueueOut.back();
		BuildingQueueOut.pop_back();
		auto position = elem->GetPosition();
		if (chunks.find(index3D(position.x, position.y, position.z)) ==
		    chunks.end())
			PushChunk(position, elem);
		else {
			// Race condition: the element was built twice
			delete elem;
		}
	}
	queueOutMutex.unlock();


	auto it = std::begin(chunks);
	while (it != std::end(chunks)) {
		if (fabs(std::get<0>(it->first) - chunkPosition.x) >
		        chunkMaxDistance * CHUNK_SIZE ||
		    fabs(std::get<2>(it->first) - chunkPosition.z) >
		        chunkMaxDistance * CHUNK_SIZE) {
			delete it->second;
			it = chunks.erase(it);
		} else {
			++it;
		}
	}

	bool buildNeeded = false;

	queueInMutex.lock();
	for (int x = -chunkMaxDistance; x < chunkMaxDistance; x++) {
		for (int z = chunkMaxDistance; z >= -chunkMaxDistance; z--) {
			glm::vec3 cursor(chunkPosition.x + (x * CHUNK_SIZE), 0,
			                 chunkPosition.z + (z * CHUNK_SIZE));
			if (!frustrum->IsPointIn(
			        glm::vec3(CHUNK_SIZE / 2, CHUNK_SIZE / 2, CHUNK_SIZE / 2) +
			        cursor))
				continue;
			bool isBeingBuilt = false;
			if (chunks.find(index3D(cursor.x, cursor.y, cursor.z)) !=
			    chunks.end())
				continue;

			for (auto args : BuildingQueueIn) {
				if (args.pos == cursor) {
					isBeingBuilt = true;
					break;
				}
			}
			if (!isBeingBuilt) {
				buildNeeded = true;
				chunkArguments args = {eBiome::MOUNTAIN, cursor};
				this->BuildingQueueIn.push_back(args);
			}
		}
	}
	queueInMutex.unlock();
	if (buildNeeded)
		buildCondition.notify_one();


	ImGui::Text("Chunk Position: %f %f %f\n", chunkPosition.x, chunkPosition.y,
	            chunkPosition.z);
	ImGui::Text("Chunks loaded: %lu\n", chunks.size());
}

void ChunkManager::Draw(Shader s) {
	long long vertices_drawn = 0;
	for (auto& c : chunks) {
		if (frustrum->IsPointIn(
		        glm::vec3(std::get<0>(c.first) + CHUNK_SIZE / 2,
		                  std::get<1>(c.first) + CHUNK_SIZE / 2,
		                  std::get<2>(c.first) + CHUNK_SIZE / 2))) {
			c.second->Draw(s);
			vertices_drawn += c.second->mesh.VerticesCount;
		}
	}
	ImGui::Text("Vertices drawn: %lld", vertices_drawn);
}
