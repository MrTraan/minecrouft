#pragma once
#include <glm/glm.hpp>
#include <vector>

#include <Block.hpp>
#include <HeightMap.hpp>
#include <Mesh.hpp>
#include <Shader.hpp>
#include <constants.hpp>

constexpr size_t CHUNK_VERTICES_INITIAL_ALLOC = 3000;
constexpr size_t CHUNK_VERTICES_BATCH_ALLOC = 3000;

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

enum class eDirection {
	SOUTH,
	NORTH,
	EAST,
	WEST,
	TOP,
	BOTTOM,
};

struct Chunk {
	VoxelMesh * mesh = nullptr;
	VoxelMesh * transparentMesh = nullptr;
	bool   isDirty = false;

	// 3 dimensionnal to note cube presence, because why not
	eBlockType cubes[ CHUNK_SIZE ][ CHUNK_HEIGHT ][ CHUNK_SIZE ];

	eBiome biome = eBiome::FOREST;

	ChunkCoordinates position;

	Chunk * poolNextItem = nullptr;

	void CreateGLBuffers();
	void UpdateGLBuffers();
	void DeleteGLBuffers();

	void Destroy();
};

Chunk * preallocateChunk();
void    chunkCreateGeometry( Chunk * chunk );
void    chunkPushFace( VoxelMesh *       mesh,
                       glm::u32vec3 a,
                       glm::u32vec3 b,
                       glm::u32vec3 c,
                       glm::u32vec3 d,
                       int          width,
                       int          height,
                       eDirection   direction,
                       eBlockType   type );
