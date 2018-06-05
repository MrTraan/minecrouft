#pragma once
#include <glm/glm.hpp>
#include <vector>

constexpr int CHUNK_SIZE = 64;
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


class Chunk {
   public:
	Chunk(eBiome biome, glm::vec3 position, HeightMap* heightMap);
	~Chunk();

	void Draw(Shader shader);
	void ConstructMesh();

	void DrawCubeLine(int x, int y, int z, eDirection direction);

	u32 CountCubeLineSize(int x, int y, int z, eDirection direction);
	u32 CountMeshFaceSize();


	glm::vec3 GetPosition();

	Mesh mesh;

	static int totalBuilt;
	static int totalDestroy;

   private:
	// 3 dimensionnal to note cube presence, because why not
	eBlockType cubes[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];

	glm::vec3 position;

	eBiome biome;

	void pushFace(int x, int y, int z, eDirection direction, eBlockType type);
	void addVertex(const Vertex& v);

	u32 drawIndex = 0;
	u32 drawIndiciesIndex = 0;
};
