#include <Chunk.hpp>
#include <ChunkManager.hpp>
#include <Debug.hpp>
#include <HeightMap.hpp>

#include <imgui/imgui.h>
#include <stdlib.h>
#include <constants.hpp>

eDirection oppositeDirection(eDirection dir) {
	switch (dir)
	{
	case FRONT:
		return BACK;
	case RIGHT:
		return LEFT;
	case BACK:
		return FRONT;
	case LEFT:
		return RIGHT;
	case TOP:
		return BOTTOM;
	case BOTTOM:
		return TOP;
	}
}

Chunk::Chunk(chunkArguments args)
    : position(args.pos), worldPosition((float)args.pos.x * CHUNK_SIZE, 0.0f, (float)args.pos.y * CHUNK_SIZE) {
	auto& heightMap = ChunkManager::instance->heightMap;

	for (u32 i = 0; i < CHUNK_SIZE; i++) {
		for (u32 k = 0; k < CHUNK_SIZE; k++) {
			u32 seed =
			    heightMap.GetValue(i + (u32)worldPosition.x, k + (s32)worldPosition.z);
			for (u32 j = 0; j < seed; j++) {
				if (biome == eBiome::FOREST)
					this->cubes[i][j][k] = eBlockType::GRASS;
				else if (biome == eBiome::MOUNTAIN)
					this->cubes[i][j][k] = eBlockType::SNOW;
			}
			for (u32 j = seed; j < CHUNK_HEIGHT; j++)
				this->cubes[i][j][k] = eBlockType::INACTIVE;
		}
	}
	
	leftNeighbor = args.leftNeighbor;
	rightNeighbor = args.rightNeighbor;
	frontNeighbor = args.frontNeighbor;
	backNeighbor = args.backNeighbor;

	this->biome = biome;
	this->ConstructMesh();

}

Chunk::~Chunk() {
	if (mesh.Vertices)
		delete[] mesh.Vertices;
	if (mesh.Indices)
		delete[] mesh.Indices;
}

void Chunk::pushFace(int x,
                     int y,
                     int z,
                     eDirection direction,
                     eBlockType type) {
	glm::vec2 uvModifier = glm::vec2(0, type == eBlockType::SNOW ? 0.5f : 0);
	u32 currentSize = drawIndex;
	Vertex v;

	if (direction == eDirection::FRONT) {
		v.Position = glm::vec3(x, y, z) + worldPosition;
		v.TexCoords = glm::vec2(0.25f, 0.5f) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
		v.Position.x += 1.0f;
		v.TexCoords = glm::vec2(0.5f, 0.5f) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
		v.Position.y += 1.0f;
		v.TexCoords = glm::vec2(0.5f, 1.0f) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
		v.Position.x -= 1.0f;
		v.TexCoords = glm::vec2(0.25f, 1.0f) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
	}

	if (direction == eDirection::BACK) {
		v.Position = glm::vec3(x, y, z + 1.0f) + worldPosition;
		v.TexCoords = glm::vec2(0.25f, 0.5f) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
		v.Position.x += 1.0f;
		v.TexCoords = glm::vec2(0.5f, 0.5f) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
		v.Position.y += 1.0f;
		v.TexCoords = glm::vec2(0.5f, 1.0f) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
		v.Position.x -= 1.0f;
		v.TexCoords = glm::vec2(0.25f, 1.0f) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
	}

	if (direction == eDirection::TOP) {
		v.Position = glm::vec3(x, y + 1, z) + worldPosition;
		v.TexCoords = glm::vec2(0.0f, 0.5f) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
		v.Position.x += 1.0f;
		v.TexCoords = glm::vec2(0.25f, 0.5f) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
		v.Position.z += 1.0f;
		v.TexCoords = glm::vec2(0.25f, 1.0f) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
		v.Position.x -= 1.0f;
		v.TexCoords = glm::vec2(0.0f, 1.0f) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
	}

	if (direction == eDirection::BOTTOM) {
		v.Position = glm::vec3(x, y, z) + worldPosition;
		v.TexCoords = glm::vec2(0.75f, 0.5f) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
		v.Position.x += 1.0f;
		v.TexCoords = glm::vec2(1.0f, 0.5f) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
		v.Position.z += 1.0f;
		v.TexCoords = glm::vec2(1.0f, 1.0f) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
		v.Position.x -= 1.0f;
		v.TexCoords = glm::vec2(0.75f, 1.0f) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
	}

	if (direction == eDirection::LEFT) {
		v.Position = glm::vec3(x, y, z) + worldPosition;
		v.TexCoords = glm::vec2(0.25f, 0.5f) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
		v.Position.z += 1.0f;
		v.TexCoords = glm::vec2(0.5f, 0.5f) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
		v.Position.y += 1.0f;
		v.TexCoords = glm::vec2(0.5f, 1.0f) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
		v.Position.z -= 1.0f;
		v.TexCoords = glm::vec2(0.25f, 1.0f) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
	}

	if (direction == eDirection::RIGHT) {
		v.Position = glm::vec3(x + 1, y, z) + worldPosition;
		v.TexCoords = glm::vec2(0.25f, 0.5f) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
		v.Position.z += 1.0f;
		v.TexCoords = glm::vec2(0.5f, 0.5f) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
		v.Position.y += 1.0f;
		v.TexCoords = glm::vec2(0.5f, 1.0f) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
		v.Position.z -= 1.0f;
		v.TexCoords = glm::vec2(0.25f, 1.0f) + uvModifier;
		mesh.Vertices[drawIndex++] = v;
	}

	mesh.Indices[drawIndiciesIndex++] = currentSize + 0;
	mesh.Indices[drawIndiciesIndex++] = currentSize + 1;
	mesh.Indices[drawIndiciesIndex++] = currentSize + 2;
	mesh.Indices[drawIndiciesIndex++] = currentSize + 0;
	mesh.Indices[drawIndiciesIndex++] = currentSize + 2;
	mesh.Indices[drawIndiciesIndex++] = currentSize + 3;
}


void Chunk::DrawCubeLine(int x, int y, int z, eDirection direction) {
	if (direction != TOP && direction != BOTTOM && cubes[x][y][z] != eBlockType::INACTIVE) {
		if (direction == LEFT && leftNeighbor) {
			if (leftNeighbor->cubes[CHUNK_SIZE - 1][y][z] == eBlockType::INACTIVE)
				pushFace(x, y, z, direction, this->cubes[x][y][z]);
		}
		else if (direction == LEFT) {
			pushFace(x, y, z, direction, this->cubes[x][y][z]);
		}

		if (direction == RIGHT && rightNeighbor) {
			if (rightNeighbor->cubes[0][y][z] == eBlockType::INACTIVE)
				pushFace(x, y, z, direction, this->cubes[x][y][z]);
		}
		else if (direction == RIGHT) {
			pushFace(x, y, z, direction, this->cubes[x][y][z]);
		}

		if (direction == FRONT && frontNeighbor) {
			if (frontNeighbor->cubes[x][y][CHUNK_SIZE - 1] == eBlockType::INACTIVE)
				pushFace(x, y, z, direction, this->cubes[x][y][z]);
		}
		else if (direction == FRONT) {
			pushFace(x, y, z, direction, this->cubes[x][y][z]);
		}

		if (direction == BACK && backNeighbor) {
			if (backNeighbor->cubes[x][y][0] == eBlockType::INACTIVE)
				pushFace(x, y, z, direction, this->cubes[x][y][z]);
		}
		else if (direction == BACK) {
			pushFace(x, y, z, direction, this->cubes[x][y][z]);
		}

	} else if (this->cubes[x][y][z]) {
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
	if (direction != TOP && direction != BOTTOM && cubes[x][y][z] != eBlockType::INACTIVE) {
		if (direction == LEFT && leftNeighbor) {
			if (leftNeighbor->cubes[CHUNK_SIZE - 1][y][z] == eBlockType::INACTIVE)
				total++;
		}
		else if (direction == LEFT) {
			total++;
		}

		if (direction == RIGHT && rightNeighbor) {
			if (rightNeighbor->cubes[0][y][z] == eBlockType::INACTIVE)
				total++;
		}
		else if (direction == RIGHT) {
			total++;
		}

		if (direction == FRONT && frontNeighbor) {
			if (frontNeighbor->cubes[x][y][CHUNK_SIZE - 1] == eBlockType::INACTIVE)
				total++;
		}
		else if (direction == FRONT) {
			total++;
		}

		if (direction == BACK && backNeighbor) {
			if (backNeighbor->cubes[x][y][0] == eBlockType::INACTIVE)
				total++;
		}
		else if (direction == BACK) {
			total++;
		}

	} else if (this->cubes[x][y][z]) {
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
	drawIndiciesIndex = 0;

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
