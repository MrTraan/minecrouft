#pragma once
#include <glm/glm.hpp>
#include <vector>

constexpr int CHUNK_SIZE = 16;
constexpr int CHUNK_HEIGHT = 256;

#include <HeightMap.hpp>
#include <Mesh.hpp>
#include <Shader.hpp>
#include <Texture.hpp>
#include <TextureManager.hpp>

enum eBiome {
	FOREST,
	MOUNTAIN,
};

enum eBlockType : char { INACTIVE = 0, GRASS, SNOW };

enum eDirection {
	FRONT = 0,
	RIGHT = 1,
	BACK = 2,
	LEFT = 3,
	TOP = 4,
	BOTTOM = 5,
};

struct chunkArguments {
	eBiome biome;
	glm::i32vec2 pos;
};

class Chunk {
   public:
	Chunk(chunkArguments args);
	~Chunk();

	void Draw(Shader shader);
	void ConstructMesh();

	void DrawCubeLine(int x, int y, int z, eDirection direction);

	u32 CountCubeLineSize(int x, int y, int z, eDirection direction);
	u32 CountMeshFaceSize();


	Mesh mesh;
	
	glm::i32vec2 position;
	glm::i32vec3 worldPosition;

	// 3 dimensionnal to note cube presence, because why not
	eBlockType cubes[CHUNK_SIZE][CHUNK_HEIGHT][CHUNK_SIZE];


	eBiome biome;

	void pushFace(int x, int y, int z, eDirection direction, eBlockType type);

	u32 drawIndex = 0;
	u32 drawIndicesIndex = 0;
};
