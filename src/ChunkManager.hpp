#pragma once

#include <Chunk.hpp>
#include <Frustrum.hpp>
#include <Shader.hpp>
#include <condition_variable>
#include <glm/glm.hpp>
#include <list>
#include <map>
#include <mutex>
#include <thread>
#include <tracy/Tracy.hpp>

#define NUM_MANAGER_THREADS 3

struct Camera;

glm::i32vec2 WorldToChunkPosition( const glm::vec3 & playerPos );
glm::vec3    ChunkToWorldPosition( const glm::i32vec2 & pos );
glm::vec3    ChunkToWorldPosition( ChunkCoordinates pos );

class ChunkManager {
  public:
	ChunkManager( glm::vec3 playerPos, Frustrum * frustrum );
	~ChunkManager();

	Shader shader;

	int  chunkLoadRadius = 6;
	int  chunkUnloadRadius = 12;
	bool ThreadShouldRun = true;

	void Update( glm::vec3 playerPos );
	void Draw( const Camera & camera );
	bool PushChunkToProducer( ChunkCoordinates coord );

	inline bool ChunkIsLoaded( ChunkCoordinates pos );
	inline bool ChunkIsLoaded( u16 x, u16 y );

	std::map< ChunkCoordinates, Chunk * > chunks;
	Frustrum *                            frustrum;
	HeightMap                             heightMap;

	glm::i32vec2 lastPosition;

	std::condition_variable updateCondition;
	std::mutex              ucMutex;

	std::thread builderRoutineThreads[ NUM_MANAGER_THREADS ];

	Chunk * poolHead = nullptr;
	Chunk * popChunkFromPool();
	void    pushChunkToPool( Chunk * item );
};
