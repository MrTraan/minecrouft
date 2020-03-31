#pragma once
#include <glm/glm.hpp>
#include <vector>

#include <HeightMap.hpp>
#include <Mesh.hpp>
#include <Shader.hpp>
#include <constants.hpp>

// To optimize memory allocation, memory for 8192 faces are created right away
// Because profiling showed that no chunk had under 1024 faces
// Faces are then allocated 1024 by 1024 when needed
constexpr size_t FACES_INITIAL_ALLOC = 3000;
constexpr size_t FACES_BATCH_ALLOC = 500;

constexpr int   TEXTURE_ROWS = 4;
constexpr float UV_Y_BASE = ( 1.0f / TEXTURE_ROWS );

// First 16 bits: x coordinate, next 16 bits: z coordinates
struct ChunkCoordinates {
	u16 x;
	u16 z;

	bool operator==( const ChunkCoordinates & c ) const { return c.x == x && c.z == z; }
	bool operator<( const ChunkCoordinates & c ) const {
		u32 old = ( z << 16 ) | x;
		u32 cold = ( c.z << 16 ) | c.x;
		return old < cold;
	}
};

constexpr u16 CHUNK_MAX_X = 16000;
constexpr u16 CHUNK_MAX_Z = 16000;

static inline ChunkCoordinates createChunkCoordinates( u16 x, u16 z ) {
	ChunkCoordinates c = {x, z};
	return c;
}
static inline u16 getXCoord( ChunkCoordinates coord ) { return coord.x; }
static inline u16 getZCoord( ChunkCoordinates coord ) { return coord.z; }

enum class eBiome {
	FOREST,
	MOUNTAIN,
};

enum class eBlockType : char { INACTIVE = 0, GRASS, SAND, DIRT, ROCK, SNOW };

enum class eDirection {
	SOUTH,
	NORTH,
	EAST,
	WEST,
	TOP,
	BOTTOM,
};

struct Chunk {
	Mesh * mesh = nullptr;
	u32    facesAllocated = 0;
	u32    facesBuilt = 0;

	// 3 dimensionnal to note cube presence, because why not
	eBlockType cubes[ CHUNK_SIZE ][ CHUNK_HEIGHT ][ CHUNK_SIZE ];

	eBiome biome;

	ChunkCoordinates position;

	Chunk * poolNextItem;
};

Chunk * preallocateChunk();
void    chunkCreateGeometry( Chunk * chunk );
void    chunkDraw( Chunk * chunk );
void    chunkDestroy( Chunk * chunk );
