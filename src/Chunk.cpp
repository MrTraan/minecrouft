#include <Chunk.hpp>
#include <Debug.hpp>
#include <HeightMap.hpp>

Chunk::Chunk(eBiome biome, glm::vec3 position, HeightMap* heightMap)
    : position(position) {
	for (int i = 0; i < CHUNK_SIZE; i++) {
		for (int k = 0; k < CHUNK_SIZE; k++) {
			int seed = heightMap->GetValue(i, k);
			for (int j = 0; j < seed; j++)
				this->cubes[i][j][k] = true;
			for (int j = seed; j < CHUNK_SIZE; j++)
				this->cubes[i][j][k] = false;
		}
	}

	this->biome = biome;
	this->ConstructMesh();
}

Chunk::~Chunk() {
	printf("Destroying chunk\n");
}

glm::vec3 Chunk::GetPosition() {
	return this->position;
}

void Chunk::pushFace(int x, int y, int z, eDirection direction) {
	int currentSize = this->mesh.Vertices.size();
	Vertex v;

	if (direction == eDirection::FRONT) {
		v.Position = glm::vec3(x, y, z) + this->position;
		v.TexCoords = glm::vec2(0.25f, 0.333334f);
		this->mesh.Vertices.push_back(v);
		v.Position.x += 1.0f;
		v.TexCoords = glm::vec2(0.5f, 0.333334f);
		this->mesh.Vertices.push_back(v);
		v.Position.y += 1.0f;
		v.TexCoords = glm::vec2(0.5f, 0.666667f);
		this->mesh.Vertices.push_back(v);
		v.Position.x -= 1.0f;
		v.TexCoords = glm::vec2(0.25f, 0.666667f);
		this->mesh.Vertices.push_back(v);
	}

	if (direction == eDirection::BACK) {
		v.Position = glm::vec3(x, y, z + 1.0f) + this->position;
		v.TexCoords = glm::vec2(0.5f, 0.333334f);
		this->mesh.Vertices.push_back(v);
		v.Position.x += 1.0f;
		v.TexCoords = glm::vec2(0.25f, 0.333334f);
		this->mesh.Vertices.push_back(v);
		v.Position.y += 1.0f;
		v.TexCoords = glm::vec2(0.25f, 0.666667f);
		this->mesh.Vertices.push_back(v);
		v.Position.x -= 1.0f;
		v.TexCoords = glm::vec2(0.5f, 0.666667f);
		this->mesh.Vertices.push_back(v);
	}

	if (direction == eDirection::TOP) {
		v.Position = glm::vec3(x, y + 1, z) + this->position;
		v.TexCoords = glm::vec2(0.25f, 1.0f);
		this->mesh.Vertices.push_back(v);
		v.Position.x += 1.0f;
		v.TexCoords = glm::vec2(0.5f, 1.0f);
		this->mesh.Vertices.push_back(v);
		v.Position.z += 1.0f;
		v.TexCoords = glm::vec2(0.5f, 0.6666667f);
		this->mesh.Vertices.push_back(v);
		v.Position.x -= 1.0f;
		v.TexCoords = glm::vec2(0.25f, 0.666667f);
		this->mesh.Vertices.push_back(v);
	}

	if (direction == eDirection::BOTTOM) {
		v.Position = glm::vec3(x, y, z) + this->position;
		v.TexCoords = glm::vec2(0.25f, 0.0f);
		this->mesh.Vertices.push_back(v);
		v.Position.x += 1.0f;
		v.TexCoords = glm::vec2(0.5f, 0.0f);
		this->mesh.Vertices.push_back(v);
		v.Position.z += 1.0f;
		v.TexCoords = glm::vec2(0.5f, 0.333334f);
		this->mesh.Vertices.push_back(v);
		v.Position.x -= 1.0f;
		v.TexCoords = glm::vec2(0.25f, 0.333334f);
		this->mesh.Vertices.push_back(v);
	}

	if (direction == eDirection::LEFT) {
		v.Position = glm::vec3(x, y, z) + this->position;
		v.TexCoords = glm::vec2(0.25f, 0.333334f);
		this->mesh.Vertices.push_back(v);
		v.Position.z += 1.0f;
		v.TexCoords = glm::vec2(0.5f, 0.333334f);
		this->mesh.Vertices.push_back(v);
		v.Position.y += 1.0f;
		v.TexCoords = glm::vec2(0.5f, 0.666667f);
		this->mesh.Vertices.push_back(v);
		v.Position.z -= 1.0f;
		v.TexCoords = glm::vec2(0.25f, 0.666667f);
		this->mesh.Vertices.push_back(v);
	}

	if (direction == eDirection::RIGHT) {
		v.Position = glm::vec3(x + 1, y, z) + this->position;
		v.TexCoords = glm::vec2(0.25f, 0.333334f);
		this->mesh.Vertices.push_back(v);
		v.Position.z += 1.0f;
		v.TexCoords = glm::vec2(0.5f, 0.333334f);
		this->mesh.Vertices.push_back(v);
		v.Position.y += 1.0f;
		v.TexCoords = glm::vec2(0.5f, 0.666667f);
		this->mesh.Vertices.push_back(v);
		v.Position.z -= 1.0f;
		v.TexCoords = glm::vec2(0.25f, 0.666667f);
		this->mesh.Vertices.push_back(v);
	}

	this->mesh.Indices.push_back(currentSize + 0);
	this->mesh.Indices.push_back(currentSize + 1);
	this->mesh.Indices.push_back(currentSize + 2);
	this->mesh.Indices.push_back(currentSize + 0);
	this->mesh.Indices.push_back(currentSize + 2);
	this->mesh.Indices.push_back(currentSize + 3);
}

void Chunk::DrawCubeLine(int x, int y, int z, eDirection direction) {
	if (this->cubes[x][y][z])
		pushFace(x, y, z, direction);

	if (direction == eDirection::FRONT) {
		while (z < CHUNK_SIZE - 1) {
			if (this->cubes[x][y][z] == false && this->cubes[x][y][z + 1])
				pushFace(x, y, z + 1, direction);
			z++;
		}
	}

	if (direction == eDirection::BACK) {
		while (z >= 1) {
			if (this->cubes[x][y][z] == false && this->cubes[x][y][z - 1])
				pushFace(x, y, z - 1, direction);
			z--;
		}
	}

	if (direction == eDirection::TOP) {
		while (y >= 1) {
			if (this->cubes[x][y][z] == false && this->cubes[x][y - 1][z])
				pushFace(x, y - 1, z, direction);
			y--;
		}
	}


	if (direction == eDirection::BOTTOM) {
		while (y < CHUNK_SIZE - 1) {
			if (this->cubes[x][y][z] == false && this->cubes[x][y + 1][z])
				pushFace(x, y + 1, z, direction);
			y++;
		}
	}


	if (direction == eDirection::LEFT) {
		while (x < CHUNK_SIZE - 1) {
			if (this->cubes[x][y][z] == false && this->cubes[x + 1][y][z])
				pushFace(x + 1, y, z, direction);
			x++;
		}
	}

	if (direction == eDirection::RIGHT) {
		while (x >= 1) {
			if (this->cubes[x][y][z] == false && this->cubes[x - 1][y][z])
				pushFace(x - 1, y, z, direction);
			x--;
		}
	}
}


void Chunk::ConstructMesh() {
	eDirection dir;

	// draw front
	dir = eDirection::FRONT;
	for (int i = 0; i < CHUNK_SIZE; i++)
		for (int j = 0; j < CHUNK_SIZE; j++)
			DrawCubeLine(i, j, 0, dir);


	// draw back
	dir = eDirection::BACK;
	for (int i = 0; i < CHUNK_SIZE; i++)
		for (int j = 0; j < CHUNK_SIZE; j++)
			DrawCubeLine(i, j, CHUNK_SIZE - 1, dir);

	// draw top
	dir = eDirection::TOP;
	for (int i = 0; i < CHUNK_SIZE; i++)
		for (int k = 0; k < CHUNK_SIZE; k++)
			DrawCubeLine(i, CHUNK_SIZE - 1, k, dir);

	// draw bottom
	dir = eDirection::BOTTOM;
	for (int i = 0; i < CHUNK_SIZE; i++)
		for (int k = 0; k < CHUNK_SIZE; k++)
			DrawCubeLine(i, 0, k, dir);


	// draw left
	dir = eDirection::LEFT;
	for (int j = 0; j < CHUNK_SIZE; j++)
		for (int k = 0; k < CHUNK_SIZE; k++)
			DrawCubeLine(0, j, k, dir);

	// draw right
	dir = eDirection::RIGHT;
	for (int j = 0; j < CHUNK_SIZE; j++)
		for (int k = 0; k < CHUNK_SIZE; k++)
			DrawCubeLine(CHUNK_SIZE - 1, j, k, dir);

	this->mesh.InitMesh();
}

void Chunk::Draw(Shader shader) {
	TextureManager::GetTexture(this->biome).Bind();
	this->mesh.Draw(shader);
}
