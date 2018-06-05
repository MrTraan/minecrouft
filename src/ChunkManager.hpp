#pragma once

#include <thread>
#include <condition_variable>
#include <mutex>
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
	std::vector<chunkArguments> BuildingQueueIn;
	std::vector<Chunk *> BuildingQueueOut;


	inline HeightMap* GetHeightMap() {
		return &(this->heightMap);
	}

	inline std::mutex* GetBuilderMutex() {
		return &(this->builderMutex);
	}

	inline std::condition_variable* GetBuildCondition() {
		return &(this->buildCondition);
	}

	inline void PushChunk(glm::vec3 pos, Chunk* c) {
		this->chunks.insert(
		    std::pair<index3D, Chunk*>(index3D(pos.x, pos.y, pos.z), c));
	}
	std::mutex queueInMutex;
	std::mutex queueOutMutex;
	std::mutex builderMutex;
	std::condition_variable buildCondition;
	std::thread builderThread;
   private:
	std::map<index3D, Chunk*> chunks;
	Frustrum* frustrum;


	HeightMap heightMap;
};
