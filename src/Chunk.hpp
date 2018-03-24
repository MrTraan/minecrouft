#pragma once

#include <glm/glm.hpp>
#include <vector>

#include <Cube.hpp>
#include <Mesh.hpp>
#include <Shader.hpp>
#include <Texture.hpp>
#include <TextureManager.hpp>

constexpr int CHUNK_SIZE = 16;


class Chunk {
   public:
	Chunk(eBiome biome, glm::vec3 position);

	void Draw(Shader shader);
	void ConstructMesh();

	void DrawCubeLine(int x, int y, int z, Cube::eFaceDirection direction);

   private:
	// 3 dimensionnal xyz cube pointer arrays, because why not
	Cube* cubes[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
	Mesh mesh;

	eBiome biome;

	void pushFace(Face f);
};
