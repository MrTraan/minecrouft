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

constexpr const char * saveFilesFolderPath = "./savegames";
constexpr const char * saveFileExtension = ".save";
constexpr const char * saveFileMetaExtension = ".meta";
constexpr size_t       WORLD_NAME_MAX_SIZE = 50;

ChunkCoordinates WorldToChunkPosition( const glm::vec3 & playerPos );
glm::vec3        ChunkToWorldPosition( const glm::i32vec2 & pos );
glm::vec3        ChunkToWorldPosition( ChunkCoordinates pos );

std::string GenerateSaveFilePath( const char * worldName );
std::string GenerateMetaSaveFilePath( const char * worldName );

struct ChunkManager {
	void Init( const char * worldName, const glm::vec3 & playerPos );
	void Shutdown();

	void Update( const glm::vec3 & playerPos );
	void Draw( const Camera & camera );
	bool PushChunkToProducer( ChunkCoordinates coord );
	void CreateChunksAroundPlayer( ChunkCoordinates chunkPosition );

	bool SaveWorldToFile( const char * path, const char * metaFilePath );

	void LoadWorld( const std::string & saveFilePath, const std::string & saveFileMetaPath );
	void CreateNewWorld( const char * worldName );

	inline bool ChunkIsLoaded( ChunkCoordinates pos ) const;
	inline bool ChunkIsLoaded( u16 x, u16 y ) const;

	Chunk * GetChunkAt( ChunkCoordinates coord );

	Chunk * popChunkFromPool();
	void    pushChunkToPool( Chunk * item );

	void FlushLoadingQueue();

	void DebugDraw();

	int chunkLoadRadius = 6;
	int chunkUnloadRadius = 12;

	Shader shader;
	Shader wireframeShader;

	struct MetaChunkInfo {
		ChunkCoordinates coord;
		size_t           binaryOffset;
		size_t           binarySize;
	};

	char                                        worldName[ WORLD_NAME_MAX_SIZE ];
	std::string                                 saveFilePath;
	std::string                                 metaSaveFilePath;
	float                                       worldSeed;
	std::map< ChunkCoordinates, Chunk * >       chunks;
	std::map< ChunkCoordinates, MetaChunkInfo > chunksMetaInfo;
	HeightMap                                   heightMap;

	ChunkCoordinates lastPosition;

	std::thread builderRoutineThreads[ NUM_MANAGER_THREADS ];

	Chunk * poolHead = nullptr;

	TextureAtlas textureAtlas;

	// DEBUG
	u32  drawCallsLastFrame = 0;
	bool debugDrawOpaque = true;
	bool debugDrawTransparent = true;
	bool debugDrawWireframe = false;
};
