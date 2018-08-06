#pragma once
#include <glm/glm.hpp>
#include <vector>

#include <HeightMap.hpp>
#include <Mesh.hpp>
#include <Shader.hpp>
#include <constants.hpp>

constexpr int CHUNK_SIZE = 16;
constexpr int CHUNK_HEIGHT = 256;

// To optimize memory allocation, memory for 8192 faces are created right away
// Because profiling showed that no chunk had under 1024 faces
// Faces are then allocated 1024 by 1024 when needed
constexpr int FACES_INITIAL_ALLOC = 3000;
constexpr int FACES_BATCH_ALLOC = 500;

constexpr int TEXTURE_ROWS = 4;
constexpr float UV_Y_BASE = (1.0f / TEXTURE_ROWS);

enum eBiome {
	FOREST,
	MOUNTAIN,
};

enum eBlockType : char { INACTIVE = 0, GRASS, SAND, DIRT, ROCK, SNOW };

enum eDirection {
	FRONT = 0,
	RIGHT = 1,
	BACK = 2,
	LEFT = 3,
	TOP = 4,
	BOTTOM = 5,
};

struct Chunk {
	Mesh* mesh;

	// 3 dimensionnal to note cube presence, because why not
	eBlockType cubes[CHUNK_SIZE][CHUNK_HEIGHT][CHUNK_SIZE];

	eBiome biome;

	glm::i32vec2 position;
	glm::i32vec3 worldPosition;

	u32 facesAllocated;
	u32 facesBuilt;
};

void chunkCreateGeometry(Chunk* chunk, glm::i32vec2 position, eBiome biome, HeightMap* heightMap);
void chunkDraw(Chunk* chunk, Shader shader);
void chunkDestroy(Chunk* chunk);
