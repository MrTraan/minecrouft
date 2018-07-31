#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <constants.hpp>
#include <Shader.hpp>

struct Vertex {
	float Position[3];
	float TexCoords[2];
};

struct Mesh {
	Vertex* Vertices = NULL;
	u32 VerticesCount = 0;
	u32* Indices = NULL;
	u32 IndicesCount = 0;
	unsigned int VAO, VBO, EBO;

	bool isBound = false;
};

void meshCreateGLBuffers(Mesh* mesh);
void meshDeleteBuffers(Mesh* mesh);

void meshDraw(Mesh* mesh, Shader shader);
