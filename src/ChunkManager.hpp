#pragma once

#include <thread>
#include <condition_variable>
#include <mutex>
#include <Chunk.hpp>
#include <Frustrum.hpp>
#include <Shader.hpp>
#include <glm/glm.hpp>
#include <map>

#define NUM_MANAGER_THREADS 3

class ChunkManager {
   public:
	ChunkManager(glm::vec3 playerPos, Frustrum* frustrum);
	~ChunkManager();

	int chunkLoadRadius = 6;
	int chunkUnloadRadius = 12;
	bool ThreadShouldRun = true;

	void Update(glm::vec3 playerPos);
	void Draw(Shader s);
	glm::i32vec2 GetChunkPosition(glm::vec3 playerPos);

	inline bool ChunkIsLoaded(ChunkCoordinates pos);
	inline bool ChunkIsLoaded(u16 x, u16 y);

	std::map<ChunkCoordinates, Chunk*> chunks;
	Frustrum* frustrum;
	HeightMap heightMap;

	glm::i32vec2 lastPosition;
	glm::i32vec2 position;

	std::condition_variable updateCondition;
	std::mutex ucMutex;

	std::mutex queueInMutex;
	std::vector<ChunkCoordinates> buildingQueueIn;

	std::mutex queueOutMutex;
	std::vector<Chunk*> buildingQueueOut;

	std::thread builderRoutineThreads[NUM_MANAGER_THREADS];
};
