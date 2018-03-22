#include <Cube.hpp>

#include <stdio.h>

static std::vector<unsigned int> cubeIndices = {
    0, 1, 2, 0, 2, 3, 1, 5,  2, 2, 6,  5,  5,  7,  6,  4,  7,  5,
    3, 4, 7, 0, 4, 3, 9, 11, 8, 8, 10, 11, 12, 13, 14, 13, 14, 15,
};

static glm::vec2 cubeTextureCoords[16] = {
    glm::vec2(0.25f, 0.333334f), glm::vec2(0.5f, 0.333334f),
    glm::vec2(0.5f, 0.666667f),  glm::vec2(0.25f, 0.666667f),
    glm::vec2(0.5f, 0.333334f),  glm::vec2(0.25f, 0.333334f),
    glm::vec2(0.25f, 0.666667f), glm::vec2(0.5f, 0.666667f),
    glm::vec2(0.5f, 1.0f),       glm::vec2(0.25f, 1.0f),
    glm::vec2(0.5f, 0.6666667f), glm::vec2(0.25f, 0.6666667f),
    glm::vec2(0.5f, 0.333334f),  glm::vec2(0.25f, 0.333334f),
    glm::vec2(0.5f, 0.0f),       glm::vec2(0.25f, 0.0f)};

// store which transformation should be applied to a base position
// to get the position of a cube vertex
static glm::vec3 cubeTransform[16] = {
    glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f),
    glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),

    glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 1.0f),
    glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 1.0f),

    glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
    glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 1.0f),

    glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f),
};


Cube::Cube(glm::vec3 position) {
	// A cube has 16 vertices
	// Each vertices has a position, a normal and a texture coord
	std::vector<Vertex> vertices;

	for (int i = 0; i < 16; i++) {
		Vertex v;
		v.Position = position + cubeTransform[i];
		v.Normal = glm::vec3(1.0f, 1.0f, 1.0f);
		v.TexCoords = cubeTextureCoords[i];
		vertices.push_back(v);
	}

	this->mesh.Vertices = vertices;
	this->mesh.Indices = cubeIndices;
	this->mesh.initMesh();
}

void Cube::Draw(Shader shader) {
	this->mesh.Draw(shader);
}
