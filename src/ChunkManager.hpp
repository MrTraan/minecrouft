#pragma once

#include <Chunk.hpp>
#include <Frustrum.hpp>
#include <HeightMap.hpp>
#include <Shader.hpp>
#include <TextureAtlas.hpp>
#include <atomic>
#include <concurrentqueue.h>
#include <glm/glm.hpp>
#include <list>
#include <map>
#include <mutex>
#include <thread>
#include <tracy/Tracy.hpp>

struct Camera;
struct MegaChunk;

using MPMCChunkQueue = moodycamel::ConcurrentQueue< Chunk * >;
using MPMCMegaChunkQueue = moodycamel::ConcurrentQueue< MegaChunk * >;

constexpr const char * saveFilesFolderPath = "./savegames";
constexpr size_t       WORLD_NAME_MAX_SIZE = 50;

constexpr u32 MEGA_CHUNK_SIZE = 64;

ChunkCoordinates WorldToChunkPosition( const glm::vec3 & playerPos );
glm::vec3        ChunkToWorldPosition( const glm::i32vec2 & pos );
glm::vec3        ChunkToWorldPosition( ChunkCoordinates pos );

std::string GenerateSaveFolderPath( const char * worldName );

struct MegaChunk {
	ChunkBlocks      chunks[ MEGA_CHUNK_SIZE ][ MEGA_CHUNK_SIZE ];
	ChunkCoordinates coords;

	std::thread * thread = nullptr;

	bool isGenerated = false;
	u32  chunksInUse = 0;

	std::atomic_bool forceSave = { false };
	MPMCChunkQueue   chunksToGenerate;
	MPMCChunkQueue   chunksToUnload;

	void AssignCubesTo( Chunk & chunk );
	void CopyCubesFrom( const Chunk & chunk );
	void WriteToFile( const char * filePath );
};

struct ChunkManager {
	void Init( const char * worldName, const glm::vec3 & playerPos );
	void Shutdown();

	void Update( const glm::vec3 & playerPos );
	void Draw( const Frustrum & frustrum, u32 shadowMap );
	void DrawShadows( const Frustrum & frustrum );
	bool PushChunkToProducer( ChunkCoordinates coord );
	void CreateChunksAroundPlayer( ChunkCoordinates chunkPosition );

	void LoadWorld( const std::string & saveFileFolder );
	void CreateNewWorld( const char * worldName );

	inline bool ChunkIsLoaded( ChunkCoordinates pos ) const;
	inline bool ChunkIsLoaded( u16 x, u16 y ) const;

	Chunk * GetChunkAt( ChunkCoordinates coord );

	Chunk * popChunkFromPool();
	void    pushChunkToPool( Chunk * item );

	void FlushLoadingQueue();

	void SaveWorld();

	void DebugDraw();

	int chunkLoadRadius = 6;
	int chunkUnloadRadius = 12;

	Shader shader;
	Shader shadowShader;

	char                                  worldName[ WORLD_NAME_MAX_SIZE ];
	std::string                           saveFolderPath;
	int                                   worldSeed = 0;
	std::map< ChunkCoordinates, Chunk * > chunks;
	HeightMap                             heightMap;

	ChunkCoordinates lastPosition;

	Chunk * poolHead = nullptr;

	TextureAtlas textureAtlas;

	// DEBUG
	u32  drawCallsLastFrame = 0;
	bool debugDrawOpaque = true;
	bool debugDrawTransparent = true;
};
