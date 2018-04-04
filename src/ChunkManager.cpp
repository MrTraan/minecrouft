#include <imgui/imgui.h>
#include <ChunkManager.hpp>


#include <unistd.h>


static void* fnBuilderThread(void* vManager) {
	ChunkManager* manager = (ChunkManager*)vManager;
	std::vector<chunkArguments> todoList;
	while (1) {
		pthread_mutex_lock(manager->GetBuilderMutex());
		printf("Waiting for build condition\n");
		pthread_cond_wait(manager->GetBuildCondition(),
		                  manager->GetBuilderMutex());
		printf("Received build condition, creating todo list...\n");
		while (!manager->BuildingQueue.empty()) {
			todoList.push_back(manager->BuildingQueue.back());
			manager->BuildingQueue.pop_back();
		}
		printf("Todo  list created\n");
		pthread_mutex_unlock(manager->GetBuilderMutex());
		printf("Released mutex, creating chunks\n");
		for (auto args : todoList) {
			Chunk* c = new Chunk(args.biome, args.pos, manager->GetHeightMap());
			pthread_mutex_lock(manager->GetBuilderMutex());
			manager->PushChunk(c);
			pthread_mutex_unlock(manager->GetBuilderMutex());
		}
		printf("Done: %lu chunks created\n", todoList.size());
		todoList.clear();
	}
	return (NULL);
}


ChunkManager::ChunkManager(glm::vec3 playerPos, Frustrum* frustrum)
    : builderMutex(PTHREAD_MUTEX_INITIALIZER),
      buildCondition(PTHREAD_COND_INITIALIZER) {
	// Setting up threads
	this->frustrum = frustrum;
	glm::vec3 chunkPosition((int)(playerPos.x / CHUNK_SIZE) * CHUNK_SIZE, 0,
	                        (int)(playerPos.z / CHUNK_SIZE) * CHUNK_SIZE);

	if (playerPos.x < 0)
		chunkPosition.x -= CHUNK_SIZE;
	if (playerPos.z < 0)
		chunkPosition.z -= CHUNK_SIZE;

	this->chunks.push_back(
	    new Chunk(eBiome::FOREST, chunkPosition, &(this->heightMap)));
	int success = pthread_create(&builderThread, NULL, fnBuilderThread, this);
}

ChunkManager::~ChunkManager() {
	for (auto c : chunks)
		delete c;
}

void ChunkManager::signalBuilding() {
	pthread_cond_signal(this->GetBuildCondition());
}

void ChunkManager::Update(glm::vec3 playerPos) {
	glm::vec3 chunkPosition((int)(playerPos.x / CHUNK_SIZE) * CHUNK_SIZE, 0,
	                        (int)(playerPos.z / CHUNK_SIZE) * CHUNK_SIZE);

	if (playerPos.x < 0)
		chunkPosition.x -= CHUNK_SIZE;
	if (playerPos.z < 0)
		chunkPosition.z -= CHUNK_SIZE;

	int chunkMaxDistance = 8;

	pthread_mutex_lock(this->GetBuilderMutex());
	for (int i = this->chunks.size() - 1; i >= 0; i--) {
		glm::vec3 dist = this->chunks[i]->GetPosition() - chunkPosition;

		if (fabs(dist.x) > chunkMaxDistance * CHUNK_SIZE ||
		    fabs(dist.z) > chunkMaxDistance * CHUNK_SIZE) {
			delete this->chunks[i];
			this->chunks[i] = this->chunks.back();
			this->chunks.pop_back();
		}
	}

	bool buildNeeded = false;

	for (int x = -chunkMaxDistance; x < chunkMaxDistance; x++) {
		for (int z = -chunkMaxDistance; z < chunkMaxDistance; z++) {
			glm::vec3 cursor(chunkPosition.x + (x * CHUNK_SIZE), 0,
			                 chunkPosition.z + (z * CHUNK_SIZE));
			bool hasBeenLoaded = false;
			for (auto c : chunks) {
				if (c->GetPosition() == cursor) {
					hasBeenLoaded = true;
					break;
				}
			}
			for (auto args : BuildingQueue) {
				if (args.pos == cursor) {
					hasBeenLoaded = true;
					break;
				}
			}
			if (!hasBeenLoaded) {
				buildNeeded = true;
				this->BuildingQueue.push_back(
				    (chunkArguments){eBiome::FOREST, cursor});
			}
		}
	}
	pthread_mutex_unlock(this->GetBuilderMutex());
	if (buildNeeded)
		this->signalBuilding();


	ImGui::Text("Chunk Position: %f %f %f\n", chunkPosition.x, chunkPosition.y,
	            chunkPosition.z);
	ImGui::Text("Chunks loaded: %lu\n", chunks.size());
}

void ChunkManager::Draw(Shader s) {
	pthread_mutex_lock(this->GetBuilderMutex());
	long long vertices_drawn = 0;
	for (auto c : chunks) {
		if (frustrum->IsPointIn(glm::vec3(c->GetPosition().x + CHUNK_SIZE / 2, c->GetPosition().y + CHUNK_SIZE / 2, c->GetPosition().z + CHUNK_SIZE / 2), 1)) {
			c->Draw(s);
			vertices_drawn += c->mesh.Vertices.size();
		}
	}
	pthread_mutex_unlock(this->GetBuilderMutex());
	ImGui::Text("Vertices drawn: %lld", vertices_drawn);
}
