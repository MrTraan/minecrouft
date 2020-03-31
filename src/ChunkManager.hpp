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

#define NUM_MANAGER_THREADS 1

struct Camera;

ChunkCoordinates WorldToChunkPosition( const glm::vec3 & playerPos );
glm::vec3        ChunkToWorldPosition( const glm::i32vec2 & pos );
glm::vec3        ChunkToWorldPosition( ChunkCoordinates pos );

class ChunkManager {
  public:
	void Init( const glm::vec3 & playerPos );
	void Shutdown();

	Shader shader;

	int chunkLoadRadius = 6;
	int chunkUnloadRadius = 12;

	void Update( const glm::vec3 & playerPos );
	void Draw( const Camera & camera );
	bool PushChunkToProducer( ChunkCoordinates coord );
	void CreateChunksAroundPlayer( ChunkCoordinates chunkPosition );

	struct MetaChunkInfo {
		ChunkCoordinates coord;
		size_t           binaryOffset;
		size_t           binarySize;
	};

	bool LoadMetaDataFile( const char * metaFilePath );
	bool SaveWorldToFile( const char * path, const char * metaFilePath );
	bool LoadWorldFromFile( const char * path, const char * metaFilePath );

	inline bool ChunkIsLoaded( ChunkCoordinates pos );
	inline bool ChunkIsLoaded( u16 x, u16 y );

	std::map< ChunkCoordinates, Chunk * > chunks;
	std::map< ChunkCoordinates, MetaChunkInfo > chunksMetaInfo;
	HeightMap                             heightMap;

	FILE * chunkDataFp = nullptr;

	ChunkCoordinates lastPosition;

	std::thread builderRoutineThreads[ NUM_MANAGER_THREADS ];

	Chunk * poolHead = nullptr;
	Chunk * popChunkFromPool();
	void    pushChunkToPool( Chunk * item );

	u32  drawCallsLastFrame = 0;
	void DebugDraw();
};
