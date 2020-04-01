#pragma once

#include <Chunk.hpp>
#include <Frustrum.hpp>
#include <Shader.hpp>
#include <TextureAtlas.hpp>
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

	void Update( const glm::vec3 & playerPos );
	void Draw( const Camera & camera );
	bool PushChunkToProducer( ChunkCoordinates coord );
	void CreateChunksAroundPlayer( ChunkCoordinates chunkPosition );

	bool LoadMetaDataFile( const char * metaFilePath );
	bool SaveWorldToFile( const char * path, const char * metaFilePath );
	bool LoadWorldFromFile( const char * path, const char * metaFilePath );

	inline bool ChunkIsLoaded( ChunkCoordinates pos );
	inline bool ChunkIsLoaded( u16 x, u16 y );
	
	Chunk * GetChunkAt( ChunkCoordinates coord );
	
	Chunk * popChunkFromPool();
	void    pushChunkToPool( Chunk * item );

	void DebugDraw();

	int chunkLoadRadius = 6;
	int chunkUnloadRadius = 12;

	Shader shader;

	struct MetaChunkInfo {
		ChunkCoordinates coord;
		size_t           binaryOffset;
		size_t           binarySize;
	};


	std::map< ChunkCoordinates, Chunk * >       chunks;
	std::map< ChunkCoordinates, MetaChunkInfo > chunksMetaInfo;
	HeightMap                                   heightMap;

	FILE * chunkDataFp = nullptr;

	ChunkCoordinates lastPosition;

	std::thread builderRoutineThreads[ NUM_MANAGER_THREADS ];

	Chunk * poolHead = nullptr;

	TextureAtlas textureAtlas;

	// DEBUG
	u32  drawCallsLastFrame = 0;
	bool debugDrawOpaque = true;
	bool debugDrawTransparent = true;

};
