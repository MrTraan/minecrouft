#include <Chunk.hpp>
#include <Debug.hpp>

#include <stdlib.h>
#include <time.h>

Chunk::Chunk(eBiome biome, glm::vec3 position) {
	srand(time(NULL));

	for (int i = 0; i < CHUNK_SIZE; i++) {
		for (int j = 0; j < CHUNK_SIZE; j++) {
			for (int k = 0; k < CHUNK_SIZE; k++) {
				if (rand() % 100 < 75)
					this->cubes[i][j][k] = NULL;
				else
					this->cubes[i][j][k] =
					    new Cube(position + glm::vec3(i, j, k));
			}
		}
	}

	this->biome = biome;
	this->ConstructMesh();
}

void Chunk::pushFace(Face f) {
	int currentSize = this->mesh.Vertices.size();
	for (int i = 0; i < 4; i++)
		this->mesh.Vertices.push_back(f.vertices[i]);
	for (int i = 0; i < 6; i++)
		this->mesh.Indices.push_back(f.indices[i] + currentSize);
}

Cube* Chunk::GetFirstCube(int x, int y, int z, Cube::eFaceDirection direction) {
	if (direction == Cube::eFaceDirection::FRONT) {
		while (z < CHUNK_SIZE) {
			if (this->cubes[x][y][z] != NULL)
				return this->cubes[x][y][z];
			z++;
		}
	}
	if (direction == Cube::eFaceDirection::BACK) {
		while (z >= 0) {
			if (this->cubes[x][y][z] != NULL)
				return this->cubes[x][y][z];
			z--;
		}
	}

	if (direction == Cube::eFaceDirection::BOTTOM) {
		while (y < CHUNK_SIZE) {
			if (this->cubes[x][y][z] != NULL)
				return this->cubes[x][y][z];
			y++;
		}
	}

	if (direction == Cube::eFaceDirection::TOP) {
		while (y >= 0) {
			if (this->cubes[x][y][z] != NULL)
				return this->cubes[x][y][z];
			y--;
		}
	}


	if (direction == Cube::eFaceDirection::LEFT) {
		while (x < CHUNK_SIZE) {
			if (this->cubes[x][y][z] != NULL)
				return this->cubes[x][y][z];
			x++;
		}
	}

	if (direction == Cube::eFaceDirection::RIGHT) {
		while (x >= 0) {
			if (this->cubes[x][y][z] != NULL)
				return this->cubes[x][y][z];
			x--;
		}
	}

	return (NULL);
}


void Chunk::ConstructMesh() {
	Cube::eFaceDirection dir;

	// draw front
	dir = Cube::eFaceDirection::FRONT;
	for (int i = 0; i < CHUNK_SIZE; i++)
		for (int j = 0; j < CHUNK_SIZE; j++) {
			Cube* cube = GetFirstCube(i, j, 0, dir);
			if (cube)
				pushFace(cube->GetFace(dir));
		}

	// draw back
	dir = Cube::eFaceDirection::BACK;
	for (int i = 0; i < CHUNK_SIZE; i++)
		for (int j = 0; j < CHUNK_SIZE; j++) {
			Cube* cube = GetFirstCube(i, j, CHUNK_SIZE - 1, dir);
			if (cube)
				pushFace(cube->GetFace(dir));
		}

	// draw bottom
	dir = Cube::eFaceDirection::BOTTOM;
	for (int i = 0; i < CHUNK_SIZE; i++)
		for (int k = 0; k < CHUNK_SIZE; k++) {
			Cube* cube = GetFirstCube(i, 0, k, dir);
			if (cube)
				pushFace(cube->GetFace(dir));
		}

	// draw top
	dir = Cube::eFaceDirection::TOP;
	for (int i = 0; i < CHUNK_SIZE; i++)
		for (int k = 0; k < CHUNK_SIZE; k++) {
			Cube* cube = GetFirstCube(i, CHUNK_SIZE - 1, k, dir);
			if (cube)
				pushFace(cube->GetFace(dir));
		}

	// draw left
	dir = Cube::eFaceDirection::LEFT;
	for (int j = 0; j < CHUNK_SIZE; j++)
		for (int k = 0; k < CHUNK_SIZE; k++) {
			Cube* cube = GetFirstCube(0, j, k, dir);
			if (cube)
				pushFace(cube->GetFace(dir));
		}

	// draw right
	dir = Cube::eFaceDirection::RIGHT;
	for (int j = 0; j < CHUNK_SIZE; j++)
		for (int k = 0; k < CHUNK_SIZE; k++) {
			Cube* cube = GetFirstCube(CHUNK_SIZE - 1, j, k, dir);
			if (cube)
				pushFace(cube->GetFace(dir));
		}

	this->mesh.InitMesh();
}

void Chunk::Draw(Shader shader) {
	TextureManager::GetTexture(this->biome).Bind();
	this->mesh.Draw(shader);
}
