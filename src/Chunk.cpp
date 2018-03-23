#include <Chunk.hpp>
#include <Debug.hpp>

Chunk::Chunk(glm::vec3 position) {
	for (int i = 0; i < CHUNK_HEIGHT; i++) {
		for (int j = 0; j < CHUNK_WIDTH; j++) {
			for (int k = 0; k < CHUNK_DEPTH; k++) {
				this->cubes.push_back(new Cube(position + glm::vec3(j, i, k)));
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
	for (int i = 0; i < CHUNK_HEIGHT; i++)
		for (int j = 0; j < CHUNK_WIDTH; j++)
			pushFace(
			    this->cubes[i * CHUNK_DEPTH * CHUNK_WIDTH + j * CHUNK_WIDTH]
			        ->GetFace(Cube::eFaceDirection::FRONT));

	// draw back
	for (int i = 0; i < CHUNK_HEIGHT; i++)
		for (int j = 0; j < CHUNK_WIDTH; j++)
			pushFace(this->cubes[i * CHUNK_DEPTH * CHUNK_WIDTH +
			                     j * CHUNK_WIDTH + CHUNK_DEPTH - 1]
			             ->GetFace(Cube::eFaceDirection::BACK));

	// draw bottom
	for (int j = 0; j < CHUNK_WIDTH; j++)
		for (int k = 0; k < CHUNK_DEPTH; k++)
			pushFace(this->cubes[j * CHUNK_WIDTH + k]->GetFace(
			    Cube::eFaceDirection::BOTTOM));

	// draw top
	for (int j = 0; j < CHUNK_WIDTH; j++)
		for (int k = 0; k < CHUNK_DEPTH; k++)
			pushFace(this->cubes[j * CHUNK_WIDTH + k +
			                     (CHUNK_HEIGHT - 1) * CHUNK_WIDTH * CHUNK_DEPTH]
			             ->GetFace(Cube::eFaceDirection::TOP));

	// draw left
	for (int i = 0; i < CHUNK_HEIGHT; i++)
		for (int k = 0; k < CHUNK_DEPTH; k++)
			pushFace(this->cubes[i * CHUNK_HEIGHT * CHUNK_WIDTH + k]->GetFace(
			    Cube::eFaceDirection::LEFT));
	// draw right
	for (int i = 0; i < CHUNK_HEIGHT; i++)
		for (int k = 0; k < CHUNK_DEPTH; k++)
			pushFace(this->cubes[i * CHUNK_HEIGHT * CHUNK_WIDTH +
			                     (CHUNK_WIDTH - 1) * CHUNK_WIDTH + k]
			             ->GetFace(Cube::eFaceDirection::RIGHT));

	this->mesh.InitMesh();
}

void Chunk::Draw(Shader shader) {
	this->mesh.Draw(shader);
}
