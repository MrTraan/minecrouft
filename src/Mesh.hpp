#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "Shader.hpp"

struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
};

class Mesh {
   public:
	std::vector<Vertex> Vertices;
	std::vector<unsigned int> Indices;

	void Draw(Shader shader);
	void initMesh();

	// default constructor
	Mesh() {}
	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices);

   private:
	unsigned int VAO, VBO, EBO;
};
