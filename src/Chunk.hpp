#pragma once

#include <glm/glm.hpp>
#include <vector>

#include <Cube.hpp>
#include <Mesh.hpp>
#include <Shader.hpp>

constexpr int CHUNK_SIZE = 16;

class Chunk {
   public:
	Chunk(glm::vec3 position);

	void Draw(Shader shader);
	void ConstructMesh();

   private:
	// 3 dimensionnal xyz cube pointer arrays, because why not
	Cube* cubes[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
	Mesh mesh;

	void pushFace(Face f);
};
