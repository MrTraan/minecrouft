#pragma once

#include <Shader.hpp>
#include <constants.hpp>
#include <glm/glm.hpp>
#include <vector>

struct Vertex {
	float Position[ 3 ];
	float TexCoords[ 2 ];
	float TexIndex;
};

struct Mesh {
	Vertex *     Vertices = NULL;
	u32          VerticesCount = 0;
	u32 *        Indices = NULL;
	u32          IndicesCount = 0;
	u32          facesAllocated = 0;
	u32          facesBuilt = 0;
	unsigned int VAO, VBO, EBO;
};

void meshCreateGLBuffers( Mesh * mesh );
void meshDeleteBuffers( Mesh * mesh );
void meshUpdateBuffer( Mesh * mesh );

void meshDraw( Mesh * mesh );
