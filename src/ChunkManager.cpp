#include <imgui/imgui.h>
#include <ChunkManager.hpp>

static void* fnBuilderThread(void* vManager) {
	ChunkManager* manager = (ChunkManager*)vManager;
	std::vector<chunkArguments> todoList;
	std::unique_lock<std::mutex> lock(*(manager->GetBuilderMutex()));
	while (manager->ThreadShouldRun) {
		printf("Waiting for build condition\n");
		manager->GetBuildCondition()->wait(lock);
		printf("Received build condition, creating todo list...\n");
		manager->GetBuilderMutex()->lock();
		while (!manager->BuildingQueue.empty()) {
			todoList.push_back(manager->BuildingQueue.back());
			manager->BuildingQueue.pop_back();
		}
		printf("Todo  list created\n");
		manager->GetBuilderMutex()->unlock();
		printf("Released mutex, creating chunks\n");
		for (auto args : todoList) {
			Chunk* c = new Chunk(args.biome, args.pos, manager->GetHeightMap());
			manager->GetBuilderMutex()->lock();
			manager->PushChunk(args.pos, c);
			manager->GetBuilderMutex()->unlock();
		}
		printf("Done: %lu chunks created\n", todoList.size());
		todoList.clear();
	}
	return (NULL);
}


ChunkManager::ChunkManager(glm::vec3 playerPos, Frustrum* frustrum)
    : builderMutex(),
      buildCondition(),
	  builderThread(fnBuilderThread, this),
      ThreadShouldRun(true) {
	// Setting up threads
	this->frustrum = frustrum;
	glm::vec3 chunkPosition((int)(playerPos.x / CHUNK_SIZE) * CHUNK_SIZE, 0,
	                        (int)(playerPos.z / CHUNK_SIZE) * CHUNK_SIZE);

	if (playerPos.x < 0)
		chunkPosition.x -= CHUNK_SIZE;
	if (playerPos.z < 0)
		chunkPosition.z -= CHUNK_SIZE;

	this->PushChunk(chunkPosition, new Chunk(eBiome::FOREST, chunkPosition,
	                                         &(this->heightMap)));
}

ChunkManager::~ChunkManager() {
	this->ThreadShouldRun = false;
	GetBuilderMutex()->lock();
	for (auto& c : chunks)
		delete c.second;
}

void ChunkManager::signalBuilding() {
	this->GetBuildCondition()->notify_one();
}

void ChunkManager::Update(glm::vec3 playerPos) {
	glm::vec3 chunkPosition((int)(playerPos.x / CHUNK_SIZE) * CHUNK_SIZE, 0,
	                        (int)(playerPos.z / CHUNK_SIZE) * CHUNK_SIZE);

	if (playerPos.x < 0)
		chunkPosition.x -= CHUNK_SIZE;
	if (playerPos.z < 0)
		chunkPosition.z -= CHUNK_SIZE;

	int chunkMaxDistance = 6;

	GetBuilderMutex()->lock();

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
				chunkArguments args = { eBiome::MOUNTAIN, cursor };
				this->BuildingQueue.push_back(args);
			}
		}
	}
	GetBuilderMutex()->unlock();
	if (buildNeeded)
		this->signalBuilding();


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
			vertices_drawn += c.second->mesh.Vertices.size();
		}
	}
	GetBuilderMutex()->unlock();
	ImGui::Text("Vertices drawn: %lld", vertices_drawn);
}
