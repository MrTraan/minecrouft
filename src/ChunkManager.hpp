#pragma once

#include <thread>
#include <condition_variable>
#include <mutex>
#include <Chunk.hpp>
#include <Frustrum.hpp>
#include <Shader.hpp>
#include <glm/glm.hpp>
#include <map>

typedef std::tuple<float, float, float> index3D;

class ChunkManager {
   public:
	ChunkManager(glm::vec3 playerPos, Frustrum* frustrum);
	~ChunkManager();

	int chunkLoadRadius = 6;
	int chunkUnloadRadius = 6;
	bool ThreadShouldRun = true;

	void Update(glm::vec3 playerPos);
	void Draw(Shader s);
	glm::i32vec2 GetChunkPosition(glm::vec3 playerPos);

	bool ChunkIsLoaded(glm::i32vec2 pos);
	bool ShouldLoadChunk(glm::i32vec2 currentPos, glm::i32vec2 position);
	bool ShouldUnloadChunk(glm::i32vec2 currentPos, glm::i32vec2 position);

	std::vector<Chunk*> chunks;
	Frustrum* frustrum;
	HeightMap heightMap;

	glm::i32vec2 lastPosition;
	glm::i32vec2 position;

	std::condition_variable updateCondition;
	std::mutex ucMutex;
	std::mutex queueOutMutex;
	std::vector<Chunk*> buildingQueueOut;
	std::thread builderRoutineThread;
};
