#pragma once

#include <glm/glm.hpp>
#include <vector>

#include <Cube.hpp>
#include <Mesh.hpp>
#include <Shader.hpp>

constexpr int CHUNK_HEIGHT = 4;
constexpr int CHUNK_WIDTH = 4;
constexpr int CHUNK_DEPTH = 4;

class Chunk {
   public:
	Chunk(glm::vec3 position);

	void Draw(Shader shader);
	void ConstructMesh();

   private:
	std::vector<Cube*> cubes;
	Mesh mesh;

	void pushFace(Face f);
};
