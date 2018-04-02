#pragma once

#include <pthread.h>
#include <Chunk.hpp>
#include <Frustrum.hpp>
#include <Shader.hpp>
#include <glm/glm.hpp>

struct chunkArguments {
	eBiome biome;
	glm::vec3 pos;
};


class ChunkManager {
   public:
	ChunkManager(glm::vec3 playerPos, Frustrum* frustrum);
	~ChunkManager();

	void Update(glm::vec3 playerPos);
	void Draw(Shader s);

	pthread_mutex_t* GetBuilderMutex() {
		return &(this->builderMutex);
	}

	pthread_cond_t* GetBuildCondition() {
		return &(this->buildCondition);
	}
	std::vector<chunkArguments> BuildingQueue;

	HeightMap* GetHeightMap() {
		return &(this->heightMap);
	}

	void PushChunk(Chunk* c) {
		this->chunks.push_back(c);
	}

   private:
	pthread_t builderThread;
	pthread_mutex_t builderMutex;
	pthread_cond_t buildCondition;


	void signalBuilding();

	std::vector<Chunk*> chunks;
	Frustrum* frustrum;


	HeightMap heightMap;
};
