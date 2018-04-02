#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "Shader.hpp"

struct Vertex {
	glm::vec3 Position;
	glm::vec2 TexCoords;
};

class Mesh {
   public:
	std::vector<Vertex> Vertices;
	std::vector<unsigned int> Indices;

	void Draw(Shader shader);
	void InitMesh();

	// default constructor
	Mesh() : isInit(false) {}

   private:
	bool isInit;
	unsigned int VAO, VBO, EBO;
};
