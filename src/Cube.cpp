#include <Cube.hpp>

#include <stdio.h>
#include <Debug.hpp>

static unsigned int cubeIndices[36] = {
    // Front
    0, 1, 2, 0, 2, 3,
    // Right
    1, 5, 2, 2, 6, 5,
    // Back
    5, 7, 6, 4, 7, 5,
    // Left
    3, 4, 7, 0, 4, 3,
    // Top
    9, 11, 8, 8, 10, 11,
    // Bottom
    12, 13, 14, 13, 14, 15};

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
				Debug::PrintVec3(this->vertices[cubeIndex].Position);
				uniqueIndices[j] = cubeIndex;
				f.vertices[j] = this->vertices[cubeIndex];
				break;
			}
		}
	}

	Debug::PrintFace(f);
	return (f);
}
