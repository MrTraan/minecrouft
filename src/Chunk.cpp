#include <Chunk.hpp>
#include <Debug.hpp>

Chunk::Chunk(glm::vec3 position) {
	for (int i = 0; i < CHUNK_SIZE; i++) {
		for (int j = 0; j < CHUNK_SIZE; j++) {
			for (int k = 0; k < CHUNK_SIZE; k++) {
				this->cubes[i][j][k] = new Cube(position + glm::vec3(j, i, k));
			}
		}
	}

	this->ConstructMesh();
}

void Chunk::pushFace(Face f) {
	int currentSize = this->mesh.Vertices.size();
	for (int i = 0; i < 4; i++)
		this->mesh.Vertices.push_back(f.vertices[i]);
	for (int i = 0; i < 6; i++)
		this->mesh.Indices.push_back(f.indices[i] + currentSize);
}

void Chunk::ConstructMesh() {
	// draw front
	for (int i = 0; i < CHUNK_SIZE; i++)
		for (int j = 0; j < CHUNK_SIZE; j++)
			pushFace(
			    this->cubes[i][j][0]->GetFace(Cube::eFaceDirection::FRONT));

	// draw back
	for (int i = 0; i < CHUNK_SIZE; i++)
		for (int j = 0; j < CHUNK_SIZE; j++)
			pushFace(this->cubes[i][j][CHUNK_SIZE - 1]->GetFace(
			    Cube::eFaceDirection::BACK));

	// draw bottom
	for (int j = 0; j < CHUNK_SIZE; j++)
		for (int k = 0; k < CHUNK_SIZE; k++)
			pushFace(
			    this->cubes[0][j][k]->GetFace(Cube::eFaceDirection::BOTTOM));

	// draw top
	for (int j = 0; j < CHUNK_SIZE; j++)
		for (int k = 0; k < CHUNK_SIZE; k++)
			pushFace(this->cubes[CHUNK_SIZE - 1][j][k]->GetFace(
			    Cube::eFaceDirection::TOP));

	// draw left
	for (int i = 0; i < CHUNK_SIZE; i++)
		for (int k = 0; k < CHUNK_SIZE; k++)
			pushFace(this->cubes[i][0][k]->GetFace(Cube::eFaceDirection::LEFT));
	// draw right
	for (int i = 0; i < CHUNK_SIZE; i++)
		for (int k = 0; k < CHUNK_SIZE; k++)
			pushFace(this->cubes[i][CHUNK_SIZE - 1][k]->GetFace(
			    Cube::eFaceDirection::RIGHT));

	this->mesh.InitMesh();
}

void Chunk::Draw(Shader shader) {
	this->mesh.Draw(shader);
}
