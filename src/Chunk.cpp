#include <Chunk.hpp>
#include <ChunkManager.hpp>
#include <Debug.hpp>
#include <HeightMap.hpp>

#include <imgui/imgui.h>
#include <stdlib.h>
#include <constants.hpp>

Chunk::Chunk(eBiome biome, glm::i32vec2 pos, HeightMap* heightMap)
    : position(pos), worldPosition(pos.x * CHUNK_SIZE, 0, pos.y * CHUNK_SIZE) {

	heightMap->SetupChunk(this);
	this->biome = biome;
	this->ConstructMesh();

}

#define TEXTURE_ROWS 4
#define UV_Y_BASE (1.0f / TEXTURE_ROWS)

Chunk::~Chunk() {
	if (mesh.Vertices)
		delete[] mesh.Vertices;
	if (mesh.Indices)
		delete[] mesh.Indices;
}

void Chunk::pushFace(s32 x,
                     s32 y,
                     s32 z,
                     eDirection direction,
                     eBlockType type) {
	glm::vec2 uvModifier(0.0f, 0.0f);
	switch (type) {
	case eBlockType::ROCK:
		uvModifier.y = UV_Y_BASE;
		break;
	case eBlockType::SNOW:
		uvModifier.y = 2 * UV_Y_BASE;
	case eBlockType::GRASS:
		uvModifier.y = 3 * UV_Y_BASE;
	}

	u32 currentSize = drawIndex;
	Vertex v;

	if (direction == eDirection::FRONT) {
		v.Position = glm::i32vec3(x, y, z) + worldPosition;
		v.TexCoords = glm::vec2(.25f, 0.0f) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
		v.Position.x += 1.0f;
		v.TexCoords = glm::vec2(.5f, 0.0f) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
		v.Position.y += 1.0f;
		v.TexCoords = glm::vec2(.5f, UV_Y_BASE) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
		v.Position.x -= 1.0f;
		v.TexCoords = glm::vec2(.25f, UV_Y_BASE) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
	}

	if (direction == eDirection::BACK) {
		v.Position = glm::i32vec3(x, y, z + 1.0f) + worldPosition;
		v.TexCoords = glm::vec2(.25f, 0.0f) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
		v.Position.x += 1.0f;
		v.TexCoords = glm::vec2(.5f, 0.0f) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
		v.Position.y += 1.0f;
		v.TexCoords = glm::vec2(.5f, UV_Y_BASE) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
		v.Position.x -= 1.0f;
		v.TexCoords = glm::vec2(.25f, UV_Y_BASE) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
	}

	if (direction == eDirection::TOP) {
		v.Position = glm::i32vec3(x, y + 1, z) + worldPosition;
		v.TexCoords = glm::vec2(.0f, 0.0f) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
		v.Position.x += 1.0f;
		v.TexCoords = glm::vec2(.25f, 0.0f) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
		v.Position.z += 1.0f;
		v.TexCoords = glm::vec2(.25f, UV_Y_BASE) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
		v.Position.x -= 1.0f;
		v.TexCoords = glm::vec2(.0f, UV_Y_BASE) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
	}

	if (direction == eDirection::BOTTOM) {
		v.Position = glm::i32vec3(x, y, z) + worldPosition;
		v.TexCoords = glm::vec2(.75f, 0.0f) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
		v.Position.x += 1.0f;
		v.TexCoords = glm::vec2(1.f, 0.0f) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
		v.Position.z += 1.0f;
		v.TexCoords = glm::vec2(1.f, UV_Y_BASE) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
		v.Position.x -= 1.0f;
		v.TexCoords = glm::vec2(.75f, UV_Y_BASE) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
	}

	if (direction == eDirection::LEFT) {
		v.Position = glm::i32vec3(x, y, z) + worldPosition;
		v.TexCoords = glm::vec2(.25f, 0.0f) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
		v.Position.z += 1.0f;
		v.TexCoords = glm::vec2(.5f, 0.0f) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
		v.Position.y += 1.0f;
		v.TexCoords = glm::vec2(.5f, UV_Y_BASE) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
		v.Position.z -= 1.0f;
		v.TexCoords = glm::vec2(.25f, UV_Y_BASE) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
	}

	if (direction == eDirection::RIGHT) {
		v.Position = glm::i32vec3(x + 1, y, z) + worldPosition;
		v.TexCoords = glm::vec2(.25f, 0.0f) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
		v.Position.z += 1.0f;
		v.TexCoords = glm::vec2(.5f, 0.0f) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
		v.Position.y += 1.0f;
		v.TexCoords = glm::vec2(.5f, UV_Y_BASE) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
		v.Position.z -= 1.0f;
		v.TexCoords = glm::vec2(0.25f, UV_Y_BASE) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
	}

	mesh.Indices[drawIndicesIndex++] = currentSize + 0;
	mesh.Indices[drawIndicesIndex++] = currentSize + 1;
	mesh.Indices[drawIndicesIndex++] = currentSize + 2;
	mesh.Indices[drawIndicesIndex++] = currentSize + 0;
	mesh.Indices[drawIndicesIndex++] = currentSize + 2;
	mesh.Indices[drawIndicesIndex++] = currentSize + 3;
}


void Chunk::DrawCubeLine(s32 x, s32 y, s32 z, eDirection direction) {
	if (this->cubes[x][y][z]) {
		pushFace(x, y, z, direction, this->cubes[x][y][z]);
	}

	if (direction == eDirection::FRONT) {
		while (z < CHUNK_SIZE - 1) {
			if (this->cubes[x][y][z] == eBlockType::INACTIVE &&
			    this->cubes[x][y][z + 1])
				pushFace(x, y, z + 1, direction, this->cubes[x][y][z + 1]);
			z++;
		}
	}

	if (direction == eDirection::BACK) {
		while (z >= 1) {
			if (this->cubes[x][y][z] == eBlockType::INACTIVE &&
			    this->cubes[x][y][z - 1])
				pushFace(x, y, z - 1, direction, this->cubes[x][y][z - 1]);
			z--;
		}
	}

	if (direction == eDirection::TOP) {
		while (y >= 1) {
			if (this->cubes[x][y][z] == eBlockType::INACTIVE &&
			    this->cubes[x][y - 1][z])
				pushFace(x, y - 1, z, direction, this->cubes[x][y - 1][z]);
			y--;
		}
	}

	if (direction == eDirection::BOTTOM) {
		while (y < CHUNK_HEIGHT - 1) {
			if (this->cubes[x][y][z] == eBlockType::INACTIVE &&
			    this->cubes[x][y + 1][z])
				pushFace(x, y + 1, z, direction, this->cubes[x][y + 1][z]);
			y++;
		}
	}

	if (direction == eDirection::LEFT) {
		while (x < CHUNK_SIZE - 1) {
			if (this->cubes[x][y][z] == eBlockType::INACTIVE &&
			    this->cubes[x + 1][y][z])
				pushFace(x + 1, y, z, direction, this->cubes[x + 1][y][z]);
			x++;
		}
	}

	if (direction == eDirection::RIGHT) {
		while (x >= 1) {
			if (this->cubes[x][y][z] == eBlockType::INACTIVE &&
			    this->cubes[x - 1][y][z])
				pushFace(x - 1, y, z, direction, this->cubes[x - 1][y][z]);
			x--;
		}
	}
}

u32 Chunk::CountCubeLineSize(int x, int y, int z, eDirection direction) {
	u32 total = 0;
	if (this->cubes[x][y][z]) {
		total++;
	}

	if (direction == eDirection::FRONT) {
		while (z < CHUNK_SIZE - 1) {
			if (this->cubes[x][y][z] == eBlockType::INACTIVE &&
			    this->cubes[x][y][z + 1])
				total++;
			z++;
		}
	}

	if (direction == eDirection::BACK) {
		while (z >= 1) {
			if (this->cubes[x][y][z] == eBlockType::INACTIVE &&
			    this->cubes[x][y][z - 1])
				total++;
			z--;
		}
	}

	if (direction == eDirection::TOP) {
		while (y >= 1) {
			if (this->cubes[x][y][z] == eBlockType::INACTIVE &&
			    this->cubes[x][y - 1][z])
				total++;
			y--;
		}
	}


	if (direction == eDirection::BOTTOM) {
		while (y < CHUNK_HEIGHT - 1) {
			if (this->cubes[x][y][z] == eBlockType::INACTIVE &&
			    this->cubes[x][y + 1][z])
				total++;
			y++;
		}
	}


	if (direction == eDirection::LEFT) {
		while (x < CHUNK_SIZE - 1) {
			if (this->cubes[x][y][z] == eBlockType::INACTIVE &&
			    this->cubes[x + 1][y][z])
				total++;
			x++;
		}
	}

	if (direction == eDirection::RIGHT) {
		while (x >= 1) {
			if (this->cubes[x][y][z] == eBlockType::INACTIVE &&
			    this->cubes[x - 1][y][z])
				total++;
			x--;
		}
	}
	return total;
}

u32 Chunk::CountMeshFaceSize() {
	eDirection dir;
	u32 total = 0;

	dir = eDirection::FRONT;
	for (int x = 0; x < CHUNK_SIZE; x++)
		for (int y = 0; y < CHUNK_HEIGHT; y++)
			total += CountCubeLineSize(x, y, 0, dir);


	dir = eDirection::BACK;
	for (int x = 0; x < CHUNK_SIZE; x++)
		for (int y = 0; y < CHUNK_HEIGHT; y++)
			total += CountCubeLineSize(x, y, CHUNK_SIZE - 1, dir);

	dir = eDirection::TOP;
	for (int x = 0; x < CHUNK_SIZE; x++)
		for (int z = 0; z < CHUNK_SIZE; z++)
			total += CountCubeLineSize(x, CHUNK_HEIGHT - 1, z, dir);

	dir = eDirection::BOTTOM;
	for (int x = 0; x < CHUNK_SIZE; x++)
		for (int z = 0; z < CHUNK_SIZE; z++)
			total += CountCubeLineSize(x, 0, z, dir);


	dir = eDirection::LEFT;
	for (int y = 0; y < CHUNK_HEIGHT; y++)
		for (int z = 0; z < CHUNK_SIZE; z++)
			total += CountCubeLineSize(0, y, z, dir);

	dir = eDirection::RIGHT;
	for (int y = 0; y < CHUNK_HEIGHT; y++)
		for (int z = 0; z < CHUNK_SIZE; z++)
			total += CountCubeLineSize(CHUNK_SIZE - 1, y, z, dir);

	return total;
}

void Chunk::ConstructMesh() {
	u32 numFaces = CountMeshFaceSize();

	mesh.IndicesCount = 6 * numFaces;
	assert(IS_SIZE_T_MUL_SAFE(mesh.IndicesCount, sizeof(u32)));
	mesh.Indices = new u32[mesh.IndicesCount];

	mesh.VerticesCount = 4 * numFaces;
	assert(IS_SIZE_T_MUL_SAFE(mesh.VerticesCount, sizeof(Vertex)));
	mesh.Vertices = new Vertex[mesh.VerticesCount];

	drawIndex = 0;
	drawIndicesIndex = 0;

	eDirection dir;

	dir = eDirection::FRONT;
	for (int x = 0; x < CHUNK_SIZE; x++)
		for (int y = 0; y < CHUNK_HEIGHT; y++)
			DrawCubeLine(x, y, 0, dir);


	dir = eDirection::BACK;
	for (int x = 0; x < CHUNK_SIZE; x++)
		for (int y = 0; y < CHUNK_HEIGHT; y++)
			DrawCubeLine(x, y, CHUNK_SIZE - 1, dir);

	dir = eDirection::TOP;
	for (int x = 0; x < CHUNK_SIZE; x++)
		for (int z = 0; z < CHUNK_SIZE; z++)
			DrawCubeLine(x, CHUNK_HEIGHT - 1, z, dir);

	dir = eDirection::BOTTOM;
	for (int x = 0; x < CHUNK_SIZE; x++)
		for (int z = 0; z < CHUNK_SIZE; z++)
			DrawCubeLine(x, 0, z, dir);


	dir = eDirection::LEFT;
	for (int y = 0; y < CHUNK_HEIGHT; y++)
		for (int z = 0; z < CHUNK_SIZE; z++)
			DrawCubeLine(0, y, z, dir);

	dir = eDirection::RIGHT;
	for (int y = 0; y < CHUNK_HEIGHT; y++)
		for (int z = 0; z < CHUNK_SIZE; z++)
			DrawCubeLine(CHUNK_SIZE - 1, y, z, dir);
}

void Chunk::Draw(Shader shader) {
	TextureManager::GetTexture(eTextureFile::BLOCKS).Bind();

	this->mesh.Draw(shader);
}
