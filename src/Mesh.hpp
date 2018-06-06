#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <constants.hpp>
#include <Shader.hpp>

struct Vertex {
	glm::vec3 Position;
	glm::vec2 TexCoords;
};

class Mesh {
   public:
	Vertex* Vertices = NULL;
	u32 VerticesCount = 0;
	u32* Indices = NULL;
	u32 IndicesCount = 0;

	void Draw(Shader shader);
	void InitMesh();

	// default constructor
	Mesh() : isInit(false) {}
	~Mesh();

   private:
	bool isInit;
	unsigned int VAO, VBO, EBO;
};
