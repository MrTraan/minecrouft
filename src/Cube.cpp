#include <Cube.hpp>

#include <stdio.h>
#include <Debug.hpp>

static unsigned int cubeIndices[36] = {
    // Front
    0, 1, 2, 0, 2, 3,
    // Right
    1, 5, 6, 1, 2, 6,
    // Back
    4, 5, 6, 4, 6, 7,
    // Left
    0, 4, 7, 0, 3, 7,
    // Top
    8, 9, 11, 8, 10, 11,
    // Bottom
    12, 13, 15, 12, 14, 15};

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
	this->Init(position);
}

void Cube::Init(glm::vec3 position) {
	// A cube has 16 vertices
	// Each vertices has a position, a normal and a texture coord
	for (int i = 0; i < 16; i++) {
		this->vertices[i].Position = position + cubeTransform[i];
		this->vertices[i].Normal = glm::vec3(1.0f, 1.0f, 1.0f);
		this->vertices[i].TexCoords = cubeTextureCoords[i];
	}
}

Face Cube::GetFace(eFaceDirection direction) {
	Face f;
	// Get unique indices of face
	int uniqueIndices[4] = {-1, -1, -1, -1};

	f.indices[0] = 0;
	f.indices[1] = 1;
	f.indices[2] = 2;
	f.indices[3] = 0;
	f.indices[4] = 2;
	f.indices[5] = 3;

	for (int i = 0; i < 6; i++) {
		int cubeIndex = cubeIndices[i + 6 * direction];
		for (int j = 0; j < 4; j++) {
			// vertex has already been stored
			if (uniqueIndices[j] == cubeIndex)
				break;
			// write and note that that vertex has been stored
			if (uniqueIndices[j] == -1) {
				uniqueIndices[j] = cubeIndex;
				f.vertices[j] = this->vertices[cubeIndex];
				break;
			}
		}
	}

	return (f);
}
