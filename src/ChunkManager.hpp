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

struct chunkArguments {
	eBiome biome;
	glm::i32vec2 pos;
};

class ChunkManager {
   public:
	ChunkManager(glm::vec3 playerPos, Frustrum* frustrum);
	~ChunkManager();

	std::mutex queueInMutex;
	std::mutex queueOutMutex;
	std::mutex builderMutex;
	std::condition_variable buildCondition;
	std::thread builderThread;

	int chunkLoadRadius = 6;
	int chunkUnloadRadius = 6;
	bool ThreadShouldRun = true;

	std::vector<chunkArguments> BuildingQueueIn;
	std::vector<Chunk*> BuildingQueueOut;


	void Update(glm::vec3 playerPos);
	void Draw(Shader s);
	glm::i32vec2 GetChunkPosition(glm::vec3 playerPos);

	bool ChunkIsLoaded(glm::i32vec2 pos);
	bool ChunkIsLoaded(s32 ax, s32 ay);
	bool ShouldLoadChunk(glm::i32vec2 currentPos, glm::i32vec2 position);
	bool ShouldUnloadChunk(glm::i32vec2 currentPos, glm::i32vec2 position);

	static ChunkManager* instance;

	std::vector<Chunk*> chunks;
	Frustrum* frustrum;
	HeightMap heightMap;
};
