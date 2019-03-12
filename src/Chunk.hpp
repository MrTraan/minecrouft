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
constexpr int FACES_INITIAL_ALLOC = 3000;
constexpr int FACES_BATCH_ALLOC = 500;

constexpr int TEXTURE_ROWS = 4;
constexpr float UV_Y_BASE = (1.0f / TEXTURE_ROWS);

// First 16 bits: x coordinate, next 16 bits: z coordinates
typedef u32 ChunkCoordinates;

static inline ChunkCoordinates createChunkCoordinates(u16 x, u16 z) { return (z << 16) | x; }
static inline u16 getXCoord(ChunkCoordinates coord) { return (coord) & 0xFFFF; }
static inline u16 getZCoord(ChunkCoordinates coord) { return (coord >> 16) & 0xFFFF; }

enum eBiome {
	FOREST,
	MOUNTAIN,
};

enum eBlockType : char { INACTIVE = 0, GRASS, SAND, DIRT, ROCK, SNOW };

enum eDirection {
	SOUTH,
	NORTH,
	BACK,
	WEST,
	EAST,
	BOTTOM,
};

struct Chunk {
	Mesh* mesh = nullptr;
	u32 facesAllocated = 0;
	u32 facesBuilt = 0;

	// 3 dimensionnal to note cube presence, because why not
	eBlockType cubes[CHUNK_SIZE][CHUNK_HEIGHT][CHUNK_SIZE];

	eBiome biome;

	ChunkCoordinates position;
	glm::i32vec3 worldPosition;

	Chunk* poolNextItem;
};

Chunk* preallocateChunk();
void chunkCreateGeometry(Chunk* chunk, HeightMap* heightMap, eBlockType* mask);
void chunkDraw(Chunk* chunk, Shader shader);
void chunkDestroy(Chunk* chunk);
