#pragma once

#include <pthread.h>
#include <Chunk.hpp>
#include <Frustrum.hpp>
#include <Shader.hpp>
#include <glm/glm.hpp>
#include <map>

struct chunkArguments {
	eBiome biome;
	glm::vec3 pos;
};

typedef std::tuple<float, float, float> index3D;


class ChunkManager {
   public:
	ChunkManager(glm::vec3 playerPos, Frustrum* frustrum);
	~ChunkManager();

	void Update(glm::vec3 playerPos);
	void Draw(Shader s);

	bool ThreadShouldRun;
	std::vector<chunkArguments> BuildingQueue;


	inline HeightMap* GetHeightMap() {
		return &(this->heightMap);
	}

	inline pthread_mutex_t* GetBuilderMutex() {
		return &(this->builderMutex);
	}

	inline pthread_cond_t* GetBuildCondition() {
		return &(this->buildCondition);
	}

	inline void PushChunk(glm::vec3 pos, Chunk* c) {
		this->chunks.insert(
		    std::pair<index3D, Chunk*>(index3D(pos.x, pos.y, pos.z), c));
	}

   private:
	pthread_t builderThread;
	pthread_mutex_t builderMutex;
	pthread_cond_t buildCondition;


	void signalBuilding();

	std::map<index3D, Chunk*> chunks;
	Frustrum* frustrum;


	HeightMap heightMap;
};
