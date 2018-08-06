#include <Chunk.hpp>
#include <ChunkManager.hpp>
#include <Debug.hpp>
#include <HeightMap.hpp>
#include <TextureAtlas.hpp>

#include <imgui/imgui.h>
#include <stdlib.h>
#include <constants.hpp>

static void pushFace(Chunk* chunk, s32 x, s32 y, s32 z, eDirection direction, float width = 1.0f, float height = 1.0f);

#define FOREACH_CUBE_OF_CHUNK() for (u32 x = 0; x < CHUNK_SIZE; x++) \
for (u32 y = 0; y < CHUNK_HEIGHT; y++) \
for (u32 z = 0; z < CHUNK_SIZE; z++) \

void chunkCreateGeometry(Chunk* chunk, glm::i32vec2 pos, eBiome biome, HeightMap* heightMap)
{
	chunk->position = pos;
	chunk->worldPosition = glm::i32vec3(pos.x * CHUNK_SIZE, 0, pos.y * CHUNK_SIZE);
	chunk->biome = biome;

	heightMap->SetupChunk(chunk);

	chunk->mesh = new Mesh();
	chunk->mesh->isBound = false;
	chunk->mesh->Indices = (u32*)malloc(sizeof(u32) * 6 * FACES_INITIAL_ALLOC);
	assert(chunk->mesh->Indices != NULL);
	chunk->mesh->Vertices = (Vertex*)malloc(sizeof(Vertex) * 4 * FACES_INITIAL_ALLOC);
	assert(chunk->mesh->Vertices != NULL);
	chunk->mesh->IndicesCount = 0;
	chunk->mesh->VerticesCount = 0;

	chunk->facesAllocated = FACES_INITIAL_ALLOC;
	chunk->facesBuilt = 0;
	
	bool mask[CHUNK_SIZE][CHUNK_HEIGHT][CHUNK_SIZE];

	for (int i = 0; i < 2; i++)
	{
		// zero out mask
		for (u32 x = 0; x < CHUNK_SIZE; x++)
			for (u32 y = 0; y < CHUNK_HEIGHT; y++)
				for (u32 z = 0; z < CHUNK_SIZE; z++)
					memset(mask[x][y], 0, CHUNK_SIZE);

		auto dir = (i == 0 ? eDirection::LEFT : eDirection::RIGHT);

		FOREACH_CUBE_OF_CHUNK()
		{
			if (mask[x][y][z] || chunk->cubes[x][y][z] == eBlockType::INACTIVE)
				continue;

			if ((dir == eDirection::LEFT && (x == 0 || (x > 0 && chunk->cubes[x - 1][y][z] == eBlockType::INACTIVE))) ||
				(x == CHUNK_SIZE - 1 || (x < CHUNK_SIZE - 1 && chunk->cubes[x + 1][y][z] == eBlockType::INACTIVE)))
			{
				mask[x][y][z] = true;
				int width = 1;
				while (z + width < CHUNK_SIZE && mask[x][y][z + width] == false && chunk->cubes[x][y][z + width] == chunk->cubes[x][y][z])
				{
					mask[x][y][z + width] = true;
					width++;
				}
				int height = 1;
				while (y + height < CHUNK_HEIGHT)
				{
					for (u32 zz = z; zz < z + width; zz++)
					{
						if (chunk->cubes[x][y + height][zz] != chunk->cubes[x][y][zz] || mask[x][y + height][zz] == true)
							goto PUSH_FACE_X;
					}
					for (u32 zz = z; zz < z + width; zz++)
						mask[x][y + height][zz] = true;
					height++;
				}
			PUSH_FACE_X:
				pushFace(chunk, x, y, z, dir, (float)width, (float)height);
			}
		}
	}

	for (int i = 0; i < 2; i++)
	{
		// zero out mask
		for (u32 x = 0; x < CHUNK_SIZE; x++)
			for (u32 y = 0; y < CHUNK_HEIGHT; y++)
				for (u32 z = 0; z < CHUNK_SIZE; z++)
					memset(mask[x][y], 0, CHUNK_SIZE);

		auto dir = (i == 0 ? eDirection::BOTTOM : eDirection::TOP);

		FOREACH_CUBE_OF_CHUNK()
		{
			if (mask[x][y][z] || chunk->cubes[x][y][z] == eBlockType::INACTIVE)
				continue;

			if ((dir == eDirection::BOTTOM && (y == 0 || (y > 0 && chunk->cubes[x][y - 1][z] == eBlockType::INACTIVE))) ||
				(y == CHUNK_HEIGHT - 1 || (y < CHUNK_HEIGHT - 1) && chunk->cubes[x][y + 1][z] == eBlockType::INACTIVE))
			{
				mask[x][y][z] = true;
				int width = 1;
				while (x + width < CHUNK_SIZE && mask[x + width][y][z] == false && chunk->cubes[x + width][y][z] == chunk->cubes[x][y][z])
				{
					mask[x + width][y][z] = true;
					width++;
				}
				int height = 1;
				while (z + height < CHUNK_SIZE)
				{
					for (u32 xx = x; xx < x + width; xx++)
					{
						if (chunk->cubes[xx][y][z + height] != chunk->cubes[xx][y][z] || mask[xx][y][z + height] == true)
							goto PUSH_FACE_Y;
					}
					for (u32 xx = x; xx < x + width; xx++)
						mask[xx][y][z + height] = true;
					height++;
				}
			PUSH_FACE_Y:
				pushFace(chunk, x, y, z, dir, (float)width, (float)height);
			}
		}
	}
	
	for (int i = 0; i < 2; i++)
	{
		// zero out mask
		for (u32 x = 0; x < CHUNK_SIZE; x++)
			for (u32 y = 0; y < CHUNK_HEIGHT; y++)
				for (u32 z = 0; z < CHUNK_SIZE; z++)
					memset(mask[x][y], 0, CHUNK_SIZE);

		auto dir = (i == 0 ? eDirection::FRONT : eDirection::BACK);

		FOREACH_CUBE_OF_CHUNK()
		{
			if (mask[x][y][z] || chunk->cubes[x][y][z] == eBlockType::INACTIVE)
				continue;

			if ((dir == eDirection::FRONT && (z == 0 || (z > 0 && chunk->cubes[x][y][z - 1] == eBlockType::INACTIVE))) ||
				(z == CHUNK_SIZE - 1 || (z < CHUNK_SIZE - 1 && chunk->cubes[x][y][z + 1] == eBlockType::INACTIVE)))
			{
				mask[x][y][z] = true;
				int width = 1;
				while (x + width < CHUNK_SIZE && mask[x + width][y][z] == false && chunk->cubes[x + width][y][z] == chunk->cubes[x][y][z])
				{
					mask[x + width][y][z] = true;
					width++;
				}
				int height = 1;
				while (y + height < CHUNK_HEIGHT)
				{
					for (u32 xx = x; xx < x + width; xx++)
					{
						if (chunk->cubes[xx][y + height][z] != chunk->cubes[xx][y][z] || mask[xx][y + height][z] == true)
							goto PUSH_FACE_Z;
					}
					for (u32 xx = x; xx < x + width; xx++)
						mask[xx][y + height][z] = true;
					height++;
				}
			PUSH_FACE_Z:
				pushFace(chunk, x, y, z, dir, (float)width, (float)height);
			}
		}
	}
}

void chunkDestroy(Chunk* chunk) {
	if (chunk->mesh->Vertices)
		free(chunk->mesh->Vertices);
	if (chunk->mesh->Indices)
		free(chunk->mesh->Indices);
	meshDeleteBuffers(chunk->mesh);
	delete chunk->mesh;
}


#ifdef WIN32
	#define CUBES_TEXTURE_PATH "C:\\Users\\nathan\\cpp\\minecrouft\\resources\\textures.png"
#else
	#define CUBES_TEXTURE_PATH "../resources/textures.png"
#endif

void chunkDraw(Chunk* chunk, Shader shader) {
	static TextureAtlas ta = loadTextureAtlas(CUBES_TEXTURE_PATH, 4, 4);

	bindTextureAtlas(ta);
	meshDraw(chunk->mesh, shader);
}

void pushFace(Chunk* chunk, s32 x, s32 y, s32 z, eDirection direction, float width, float height) {
	eBlockType type = chunk->cubes[x][y][z];

	int uvModifier;

	switch (type) {
	case eBlockType::ROCK:
		uvModifier = 1;
		break;
	case eBlockType::SNOW:
		uvModifier = 2;
		break;
	case eBlockType::GRASS:
		uvModifier = 3;
		break;
	default:
		uvModifier = 0;
		break;
	}


	if (chunk->facesAllocated == chunk->facesBuilt)
	{
		chunk->mesh->Indices = (u32*)realloc(chunk->mesh->Indices, sizeof(u32) * 6 * (FACES_BATCH_ALLOC + chunk->facesAllocated));
		assert(chunk->mesh->Indices != NULL);
		chunk->mesh->Vertices = (Vertex*)realloc(chunk->mesh->Vertices, sizeof(Vertex) * 4 * (FACES_BATCH_ALLOC + chunk->facesAllocated));
		assert(chunk->mesh->Vertices != NULL);
		chunk->facesAllocated += FACES_BATCH_ALLOC;
	}

	// Vertex index
	u32 vi = 4 * chunk->facesBuilt;
	// Indices index
	u32 ii = 6 * chunk->facesBuilt;

	chunk->facesBuilt++;
	
	chunk->mesh->IndicesCount += 6;
	chunk->mesh->Indices[ii + 0] = vi + 0;
	chunk->mesh->Indices[ii + 1] = vi + 1;
	chunk->mesh->Indices[ii + 2] = vi + 2;
	chunk->mesh->Indices[ii + 3] = vi + 0;
	chunk->mesh->Indices[ii + 4] = vi + 2;
	chunk->mesh->Indices[ii + 5] = vi + 3;

	chunk->mesh->VerticesCount += 4;
	Vertex* v = chunk->mesh->Vertices;

	v[vi + 0].TexCoords[0] = 0.0f;
	v[vi + 0].TexCoords[1] = 0.0f;
	v[vi + 1].TexCoords[0] = width;
	v[vi + 1].TexCoords[1] = 0.0f;
	v[vi + 2].TexCoords[0] = width;
	v[vi + 2].TexCoords[1] = height;
	v[vi + 3].TexCoords[0] = 0.0f;
	v[vi + 3].TexCoords[1] = height;

	if (direction == eDirection::FRONT)
	{
		v[vi + 0].Position[0] = chunk->worldPosition.x + x + 0.0f;
		v[vi + 0].Position[1] = chunk->worldPosition.y + y + 0.0f;
		v[vi + 0].Position[2] = chunk->worldPosition.z + z + 0.0f;
		
		v[vi + 1].Position[0] = chunk->worldPosition.x + x + width;
		v[vi + 1].Position[1] = chunk->worldPosition.y + y + 0.0f;
		v[vi + 1].Position[2] = chunk->worldPosition.z + z + 0.0f;
		
		v[vi + 2].Position[0] = chunk->worldPosition.x + x + width;
		v[vi + 2].Position[1] = chunk->worldPosition.y + y + height;
		v[vi + 2].Position[2] = chunk->worldPosition.z + z + 0.0f;
		
		v[vi + 3].Position[0] = chunk->worldPosition.x + x + 0.0f;
		v[vi + 3].Position[1] = chunk->worldPosition.y + y + height;
		v[vi + 3].Position[2] = chunk->worldPosition.z + z + 0.0f;

		for (int i = 0; i < 4; i++)
			v[vi + i].TexIndex = uvModifier * 4 + 1;
	}
	
	if (direction == eDirection::BACK)
	{
		v[vi + 0].Position[0] = chunk->worldPosition.x + x + 0.0f;
		v[vi + 0].Position[1] = chunk->worldPosition.y + y + 0.0f;
		v[vi + 0].Position[2] = chunk->worldPosition.z + z + 1.0f;
		
		v[vi + 1].Position[0] = chunk->worldPosition.x + x + width;
		v[vi + 1].Position[1] = chunk->worldPosition.y + y + 0.0f;
		v[vi + 1].Position[2] = chunk->worldPosition.z + z + 1.0f;
		
		v[vi + 2].Position[0] = chunk->worldPosition.x + x + width;
		v[vi + 2].Position[1] = chunk->worldPosition.y + y + height;
		v[vi + 2].Position[2] = chunk->worldPosition.z + z + 1.0f;
		
		v[vi + 3].Position[0] = chunk->worldPosition.x + x + 0.0f;
		v[vi + 3].Position[1] = chunk->worldPosition.y + y + height;
		v[vi + 3].Position[2] = chunk->worldPosition.z + z + 1.0f;

		for (int i = 0; i < 4; i++)
			v[vi + i].TexIndex = uvModifier * 4 + 1;
	}

	if (direction == eDirection::TOP) {
		v[vi + 0].Position[0] = chunk->worldPosition.x + x + 0.0f;
		v[vi + 0].Position[1] = chunk->worldPosition.y + y + 1.0f;
		v[vi + 0].Position[2] = chunk->worldPosition.z + z + 0.0f;
		
		v[vi + 1].Position[0] = chunk->worldPosition.x + x + width;
		v[vi + 1].Position[1] = chunk->worldPosition.y + y + 1.0f;
		v[vi + 1].Position[2] = chunk->worldPosition.z + z + 0.0f;
		
		v[vi + 2].Position[0] = chunk->worldPosition.x + x + width;
		v[vi + 2].Position[1] = chunk->worldPosition.y + y + 1.0f;
		v[vi + 2].Position[2] = chunk->worldPosition.z + z + height;
		
		v[vi + 3].Position[0] = chunk->worldPosition.x + x + 0.0f;
		v[vi + 3].Position[1] = chunk->worldPosition.y + y + 1.0f;
		v[vi + 3].Position[2] = chunk->worldPosition.z + z + height;

		for (int i = 0; i < 4; i++)
			v[vi + i].TexIndex = uvModifier * 4 + 0;
	}

	if (direction == eDirection::BOTTOM) {
		v[vi + 0].Position[0] = chunk->worldPosition.x + x + 0.0f;
		v[vi + 0].Position[1] = chunk->worldPosition.y + y + 0.0f;
		v[vi + 0].Position[2] = chunk->worldPosition.z + z + 0.0f;
		
		v[vi + 1].Position[0] = chunk->worldPosition.x + x + width;
		v[vi + 1].Position[1] = chunk->worldPosition.y + y + 0.0f;
		v[vi + 1].Position[2] = chunk->worldPosition.z + z + 0.0f;
		
		v[vi + 2].Position[0] = chunk->worldPosition.x + x + width;
		v[vi + 2].Position[1] = chunk->worldPosition.y + y + 0.0f;
		v[vi + 2].Position[2] = chunk->worldPosition.z + z + height;
		
		v[vi + 3].Position[0] = chunk->worldPosition.x + x + 0.0f;
		v[vi + 3].Position[1] = chunk->worldPosition.y + y + 0.0f;
		v[vi + 3].Position[2] = chunk->worldPosition.z + z + height;

		for (int i = 0; i < 4; i++)
			v[vi + i].TexIndex = uvModifier * 4 + 3;
	}

	if (direction == eDirection::LEFT) {
		v[vi + 0].Position[0] = chunk->worldPosition.x + x + 0.0f;
		v[vi + 0].Position[1] = chunk->worldPosition.y + y + 0.0f;
		v[vi + 0].Position[2] = chunk->worldPosition.z + z + 0.0f;
		
		v[vi + 1].Position[0] = chunk->worldPosition.x + x + 0.0f;
		v[vi + 1].Position[1] = chunk->worldPosition.y + y + 0.0f;
		v[vi + 1].Position[2] = chunk->worldPosition.z + z + width;
		
		v[vi + 2].Position[0] = chunk->worldPosition.x + x + 0.0f;
		v[vi + 2].Position[1] = chunk->worldPosition.y + y + height;
		v[vi + 2].Position[2] = chunk->worldPosition.z + z + width;
		
		v[vi + 3].Position[0] = chunk->worldPosition.x + x + 0.0f;
		v[vi + 3].Position[1] = chunk->worldPosition.y + y + height;
		v[vi + 3].Position[2] = chunk->worldPosition.z + z + 0.0f;

		for (int i = 0; i < 4; i++)
			v[vi + i].TexIndex = uvModifier * 4 + 1;
	}

	if (direction == eDirection::RIGHT) {
		v[vi + 0].Position[0] = chunk->worldPosition.x + x + 1.0f;
		v[vi + 0].Position[1] = chunk->worldPosition.y + y + 0.0f;
		v[vi + 0].Position[2] = chunk->worldPosition.z + z + 0.0f;
		
		v[vi + 1].Position[0] = chunk->worldPosition.x + x + 1.0f;
		v[vi + 1].Position[1] = chunk->worldPosition.y + y + 0.0f;
		v[vi + 1].Position[2] = chunk->worldPosition.z + z + width;
		
		v[vi + 2].Position[0] = chunk->worldPosition.x + x + 1.0f;
		v[vi + 2].Position[1] = chunk->worldPosition.y + y + height;
		v[vi + 2].Position[2] = chunk->worldPosition.z + z + width;
		
		v[vi + 3].Position[0] = chunk->worldPosition.x + x + 1.0f;
		v[vi + 3].Position[1] = chunk->worldPosition.y + y + height;
		v[vi + 3].Position[2] = chunk->worldPosition.z + z + 0.0f;

		for (int i = 0; i < 4; i++)
			v[vi + i].TexIndex = uvModifier * 4 + 1;
	}
}

