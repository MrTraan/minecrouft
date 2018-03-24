#include <Chunk.hpp>
#include <Debug.hpp>


#include <noise/noise.h>
#include <noiseutils.h>

Chunk::Chunk(eBiome biome, glm::vec3 position) {
	noise::module::Perlin gen;
	noise::utils::NoiseMap heightMap;
	noise::utils::NoiseMapBuilderPlane heightMapBuilder;

	heightMapBuilder.SetSourceModule(gen);
	heightMapBuilder.SetDestNoiseMap(heightMap);
	heightMapBuilder.SetDestSize(CHUNK_SIZE, CHUNK_SIZE);
	heightMapBuilder.SetBounds(
	    position.x / CHUNK_SIZE, position.x / CHUNK_SIZE + 1,
	    position.z / CHUNK_SIZE, position.z / CHUNK_SIZE + 1);
	heightMapBuilder.Build();

	for (int i = 0; i < CHUNK_SIZE; i++) {
		for (int k = 0; k < CHUNK_SIZE; k++) {
			int seed = (int)((heightMap.GetValue(i, k) + 1) * CHUNK_SIZE / 2);
			for (int j = 0; j < seed; j++)
				this->cubes[i][j][k] = new Cube(position + glm::vec3(i, j, k));
			for (int j = seed; j < CHUNK_SIZE; j++)
				this->cubes[i][j][k] = NULL;
		}
	}

	this->biome = biome;
	this->ConstructMesh();
}

Chunk::~Chunk() {
	for (int i = 0; i < CHUNK_SIZE; i++) {
		for (int j = 0; j < CHUNK_SIZE; j++) {
			for (int k = 0; k < CHUNK_SIZE; k++) {
				if (this->cubes[i][j][k] != NULL)
					delete (this->cubes[i][j][k]);
			}
		}
	}
}


void Chunk::pushFace(Face f) {
	int currentSize = this->mesh.Vertices.size();
	for (int i = 0; i < 4; i++)
		this->mesh.Vertices.push_back(f.vertices[i]);
	for (int i = 0; i < 6; i++)
		this->mesh.Indices.push_back(f.indices[i] + currentSize);
}

void Chunk::DrawCubeLine(int x, int y, int z, Cube::eFaceDirection direction) {
	if (this->cubes[x][y][z])
		pushFace(this->cubes[x][y][z]->GetFace(direction));

	if (direction == Cube::eFaceDirection::FRONT) {
		while (z < CHUNK_SIZE - 1) {
			if (this->cubes[x][y][z] == NULL && this->cubes[x][y][z + 1])
				pushFace(this->cubes[x][y][z + 1]->GetFace(direction));
			z++;
		}
	}

	if (direction == Cube::eFaceDirection::BACK) {
		while (z >= 1) {
			if (this->cubes[x][y][z] == NULL && this->cubes[x][y][z - 1])
				pushFace(this->cubes[x][y][z - 1]->GetFace(direction));
			z--;
		}
	}

	if (direction == Cube::eFaceDirection::BOTTOM) {
		while (y < CHUNK_SIZE - 1) {
			if (this->cubes[x][y][z] == NULL && this->cubes[x][y + 1][z])
				pushFace(this->cubes[x][y + 1][z]->GetFace(direction));
			y++;
		}
	}

	if (direction == Cube::eFaceDirection::TOP) {
		while (y >= 1) {
			if (this->cubes[x][y][z] == NULL && this->cubes[x][y - 1][z])
				pushFace(this->cubes[x][y - 1][z]->GetFace(direction));
			y--;
		}
	}


	if (direction == Cube::eFaceDirection::LEFT) {
		while (x < CHUNK_SIZE - 1) {
			if (this->cubes[x][y][z] == NULL && this->cubes[x + 1][y][z])
				pushFace(this->cubes[x + 1][y][z]->GetFace(direction));
			x++;
		}
	}

	if (direction == Cube::eFaceDirection::RIGHT) {
		while (x >= 1) {
			if (this->cubes[x][y][z] == NULL && this->cubes[x - 1][y][z])
				pushFace(this->cubes[x - 1][y][z]->GetFace(direction));
			x--;
		}
	}
}


void Chunk::ConstructMesh() {
	Cube::eFaceDirection dir;

	// draw front
	dir = Cube::eFaceDirection::FRONT;
	for (int i = 0; i < CHUNK_SIZE; i++)
		for (int j = 0; j < CHUNK_SIZE; j++)
			DrawCubeLine(i, j, 0, dir);


	// draw back
	dir = Cube::eFaceDirection::BACK;
	for (int i = 0; i < CHUNK_SIZE; i++)
		for (int j = 0; j < CHUNK_SIZE; j++)
			DrawCubeLine(i, j, CHUNK_SIZE - 1, dir);

	// draw bottom
	dir = Cube::eFaceDirection::BOTTOM;
	for (int i = 0; i < CHUNK_SIZE; i++)
		for (int k = 0; k < CHUNK_SIZE; k++)
			DrawCubeLine(i, 0, k, dir);

	// draw top
	dir = Cube::eFaceDirection::TOP;
	for (int i = 0; i < CHUNK_SIZE; i++)
		for (int k = 0; k < CHUNK_SIZE; k++)
			DrawCubeLine(i, CHUNK_SIZE - 1, k, dir);

	// draw left
	dir = Cube::eFaceDirection::LEFT;
	for (int j = 0; j < CHUNK_SIZE; j++)
		for (int k = 0; k < CHUNK_SIZE; k++)
			DrawCubeLine(0, j, k, dir);

	// draw right
	dir = Cube::eFaceDirection::RIGHT;
	for (int j = 0; j < CHUNK_SIZE; j++)
		for (int k = 0; k < CHUNK_SIZE; k++)
			DrawCubeLine(CHUNK_SIZE - 1, j, k, dir);

	this->mesh.InitMesh();
}

void Chunk::Draw(Shader shader) {
	TextureManager::GetTexture(this->biome).Bind();
	this->mesh.Draw(shader);
}
