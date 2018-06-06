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

	std::mutex queueInMutex;
	std::mutex queueOutMutex;
	std::mutex builderMutex;
	std::condition_variable buildCondition;
	std::thread builderThread;

	int chunkLoadRadius = 3;
	int chunkUnloadRadius = 6;
	bool ThreadShouldRun = true;

	std::vector<chunkArguments> BuildingQueueIn;
	std::vector<Chunk*> BuildingQueueOut;


	void Update(glm::vec3 playerPos);
	void Draw(Shader s);
	glm::vec3 GetChunkPosition(glm::vec3 pos);

	inline HeightMap* GetHeightMap() {
		return &(this->heightMap);
	}

	bool ChunkIsLoaded(glm::vec3 pos);
	bool ChunkIsLoaded(s32 ax, s32 ay, s32 az);


   private:
	std::vector<Chunk*> chunks;
	Frustrum* frustrum;
	HeightMap heightMap;
};
