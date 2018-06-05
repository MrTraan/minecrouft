#include <imgui/imgui.h>
#include <ChunkManager.hpp>

static void fnBuilderThread(ChunkManager* manager) {
	std::unique_lock<std::mutex> ulock(manager->builderMutex);
	std::vector<chunkArguments> todoList;

	while (manager->ThreadShouldRun) {
		manager->GetBuildCondition()->wait(ulock);
		printf("Received build condition, creating todo list...\n");
		printf("Waiting for build condition\n");

		manager->queueMutex.lock();
		while (!manager->BuildingQueue.empty()) {
			todoList.push_back(manager->BuildingQueue.back());
			manager->BuildingQueue.pop_back();
		}
		manager->queueMutex.unlock();
		printf("Todo  list created\n");
		printf("Released mutex, creating chunks\n");
		Chunk** outputQueue = new Chunk*[todoList.size()];
		u32 i = 0;
		for (auto args : todoList) {
			Chunk* c = new Chunk(args.biome, args.pos, manager->GetHeightMap());
			outputQueue[i++] = c;
		}
		manager->queueMutex.lock();
		for (int j = 0; j < i; j++)
			manager->PushChunk(outputQueue[j]->GetPosition(), outputQueue[j]);
		manager->queueMutex.unlock();
		printf("Done: %lu chunks created\n", todoList.size());
		todoList.clear();
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
	buildCondition.notify_one();
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

	queueMutex.lock();

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

			for (auto args : BuildingQueue) {
				if (args.pos == cursor) {
					isBeingBuilt = true;
					break;
				}
			}
			if (!isBeingBuilt) {
				buildNeeded = true;
				chunkArguments args = {eBiome::MOUNTAIN, cursor};
				this->BuildingQueue.push_back(args);
			}
		}
	}
	queueMutex.unlock();
	if (buildNeeded)
		buildCondition.notify_one();


	ImGui::Text("Chunk Position: %f %f %f\n", chunkPosition.x, chunkPosition.y,
	            chunkPosition.z);
	ImGui::Text("Chunks loaded: %lu\n", chunks.size());
}

void ChunkManager::Draw(Shader s) {
	GetBuilderMutex()->lock();
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
	GetBuilderMutex()->unlock();
	ImGui::Text("Vertices drawn: %lld", vertices_drawn);
}
