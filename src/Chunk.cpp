#include <Chunk.hpp>
#include <ChunkManager.hpp>
#include <Debug.hpp>
#include <HeightMap.hpp>

#include <imgui/imgui.h>
#include <stdlib.h>
#include <constants.hpp>

static void pushFace(Chunk* chunk, s32 x, s32 y, s32 z, eDirection direction);

void chunkCreateGeometry(Chunk* chunk, glm::i32vec2 pos, eBiome biome, HeightMap* heightMap)
{
	chunk->position = pos;
	chunk->worldPosition = glm::i32vec3(pos.x * CHUNK_SIZE, 0, pos.y * CHUNK_SIZE);
	chunk->biome = biome;

	heightMap->SetupChunk(chunk);

	chunk->mesh = new Mesh();
	chunk->mesh->isBound = false;
	chunk->mesh->Indices = (u32*)malloc(sizeof(u32) * 6 * FACES_INITIAL_ALLOC);
	chunk->mesh->Vertices = (Vertex*)malloc(sizeof(Vertex) * 4 * FACES_INITIAL_ALLOC);
	chunk->mesh->IndicesCount = 0;
	chunk->mesh->VerticesCount = 0;

	chunk->facesAllocated = FACES_INITIAL_ALLOC;
	chunk->facesBuilt = 0;
	
	u32 x = 0;
	u32 y = 0;
	u32 z = 0;
	
	for (x = 0; x < CHUNK_SIZE; x++)
	{
		for (y = 0; y < CHUNK_HEIGHT; y++)
		{
			for (z = 0; z < CHUNK_SIZE; z++)
			{
				if (x == 0 && chunk->cubes[x][y][z] != eBlockType::INACTIVE)
					pushFace(chunk, x, y, z, eDirection::LEFT);
				if (x == CHUNK_SIZE - 1 && chunk->cubes[x][y][z] != eBlockType::INACTIVE)
					pushFace(chunk, x, y, z, eDirection::RIGHT);

				if (y == 0 && chunk->cubes[x][y][z] != eBlockType::INACTIVE)
					pushFace(chunk, x, y, z, eDirection::BOTTOM);
				if (y == CHUNK_HEIGHT - 1&& chunk->cubes[x][y][z] != eBlockType::INACTIVE)
					pushFace(chunk, x, y, z, eDirection::TOP);
				
				if (z == 0 && chunk->cubes[x][y][z] != eBlockType::INACTIVE)
					pushFace(chunk, x, y, z, eDirection::FRONT);
				if (z == CHUNK_SIZE - 1 && chunk->cubes[x][y][z] != eBlockType::INACTIVE)
					pushFace(chunk, x, y, z, eDirection::BACK);
				
				if (chunk->cubes[x][y][z] == eBlockType::INACTIVE)
				{
					if (x < CHUNK_SIZE - 1 && chunk->cubes[x + 1][y][z])
						pushFace(chunk, x + 1, y, z, eDirection::LEFT);
					if (x > 0 && chunk->cubes[x - 1][y][z])
						pushFace(chunk, x - 1, y, z, eDirection::RIGHT);

					if (y < CHUNK_HEIGHT - 1 && chunk->cubes[x][y + 1][z])
						pushFace(chunk, x, y + 1, z, eDirection::BOTTOM);
					if (y > 0 && chunk->cubes[x][y - 1][z])
						pushFace(chunk, x, y - 1, z, eDirection::TOP);
					
					if (z < CHUNK_SIZE - 1 && chunk->cubes[x][y][z + 1])
						pushFace(chunk, x, y, z + 1, eDirection::FRONT);
					if (z > 0 && chunk->cubes[x][y][z - 1])
						pushFace(chunk, x, y, z - 1, eDirection::BACK);
				}
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
void chunkDraw(Chunk* chunk, Shader shader) {
	TextureManager::GetTexture(eTextureFile::BLOCKS).Bind();
	meshDraw(chunk->mesh, shader);
}

void pushFace(Chunk* chunk, s32 x, s32 y, s32 z, eDirection direction) {
	eBlockType type = chunk->cubes[x][y][z];

	float uvModifier;

	switch (type) {
	case eBlockType::ROCK:
		uvModifier = UV_Y_BASE;
		break;
	case eBlockType::SNOW:
		uvModifier = 2 * UV_Y_BASE;
		break;
	case eBlockType::GRASS:
		uvModifier = 3 * UV_Y_BASE;
		break;
	default:
		uvModifier = 0.0f;
		break;
	}


	if (chunk->facesAllocated == chunk->facesBuilt)
	{
		chunk->mesh->Indices = (u32*)realloc(chunk->mesh->Indices, sizeof(u32) * 6 * (FACES_BATCH_ALLOC + chunk->facesAllocated));
		chunk->mesh->Vertices = (Vertex*)realloc(chunk->mesh->Vertices, sizeof(Vertex) * 4 * (FACES_BATCH_ALLOC + chunk->facesAllocated));
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

	if (direction == eDirection::FRONT)
	{
		v[vi + 0].Position[0] = chunk->worldPosition.x + x + 0.0f;
		v[vi + 0].Position[1] = chunk->worldPosition.y + y + 0.0f;
		v[vi + 0].Position[2] = chunk->worldPosition.z + z + 0.0f;
		v[vi + 0].TexCoords[0] = 0.25f;
		v[vi + 0].TexCoords[1] = 0.0f + uvModifier;
		
		v[vi + 1].Position[0] = chunk->worldPosition.x + x + 1.0f;
		v[vi + 1].Position[1] = chunk->worldPosition.y + y + 0.0f;
		v[vi + 1].Position[2] = chunk->worldPosition.z + z + 0.0f;
		v[vi + 1].TexCoords[0] = 0.5f;
		v[vi + 1].TexCoords[1] = 0.0f + uvModifier;
		
		v[vi + 2].Position[0] = chunk->worldPosition.x + x + 1.0f;
		v[vi + 2].Position[1] = chunk->worldPosition.y + y + 1.0f;
		v[vi + 2].Position[2] = chunk->worldPosition.z + z + 0.0f;
		v[vi + 2].TexCoords[0] = 0.5f;
		v[vi + 2].TexCoords[1] = UV_Y_BASE + uvModifier;
		
		v[vi + 3].Position[0] = chunk->worldPosition.x + x + 0.0f;
		v[vi + 3].Position[1] = chunk->worldPosition.y + y + 1.0f;
		v[vi + 3].Position[2] = chunk->worldPosition.z + z + 0.0f;
		v[vi + 3].TexCoords[0] = 0.25f;
		v[vi + 3].TexCoords[1] = UV_Y_BASE + uvModifier;
	}
	else if (direction == eDirection::BACK)
	{
		v[vi + 0].Position[0] = chunk->worldPosition.x + x + 0.0f;
		v[vi + 0].Position[1] = chunk->worldPosition.y + y + 0.0f;
		v[vi + 0].Position[2] = chunk->worldPosition.z + z + 1.0f;
		v[vi + 0].TexCoords[0] = 0.25f;
		v[vi + 0].TexCoords[1] = 0.0f + uvModifier;
		
		v[vi + 1].Position[0] = chunk->worldPosition.x + x + 1.0f;
		v[vi + 1].Position[1] = chunk->worldPosition.y + y + 0.0f;
		v[vi + 1].Position[2] = chunk->worldPosition.z + z + 1.0f;
		v[vi + 1].TexCoords[0] = 0.5f;
		v[vi + 1].TexCoords[1] = 0.0f + uvModifier;
		
		v[vi + 2].Position[0] = chunk->worldPosition.x + x + 1.0f;
		v[vi + 2].Position[1] = chunk->worldPosition.y + y + 1.0f;
		v[vi + 2].Position[2] = chunk->worldPosition.z + z + 1.0f;
		v[vi + 2].TexCoords[0] = 0.5f;
		v[vi + 2].TexCoords[1] = UV_Y_BASE + uvModifier;
		
		v[vi + 3].Position[0] = chunk->worldPosition.x + x + 0.0f;
		v[vi + 3].Position[1] = chunk->worldPosition.y + y + 1.0f;
		v[vi + 3].Position[2] = chunk->worldPosition.z + z + 1.0f;
		v[vi + 3].TexCoords[0] = 0.25f;
		v[vi + 3].TexCoords[1] = UV_Y_BASE + uvModifier;
	}

	if (direction == eDirection::TOP) {
		v[vi + 0].Position[0] = chunk->worldPosition.x + x + 0.0f;
		v[vi + 0].Position[1] = chunk->worldPosition.y + y + 1.0f;
		v[vi + 0].Position[2] = chunk->worldPosition.z + z + 0.0f;
		v[vi + 0].TexCoords[0] = 0.0f;
		v[vi + 0].TexCoords[1] = 0.0f + uvModifier;
		
		v[vi + 1].Position[0] = chunk->worldPosition.x + x + 1.0f;
		v[vi + 1].Position[1] = chunk->worldPosition.y + y + 1.0f;
		v[vi + 1].Position[2] = chunk->worldPosition.z + z + 0.0f;
		v[vi + 1].TexCoords[0] = 0.25f;
		v[vi + 1].TexCoords[1] = 0.0f + uvModifier;
		
		v[vi + 2].Position[0] = chunk->worldPosition.x + x + 1.0f;
		v[vi + 2].Position[1] = chunk->worldPosition.y + y + 1.0f;
		v[vi + 2].Position[2] = chunk->worldPosition.z + z + 1.0f;
		v[vi + 2].TexCoords[0] = 0.25f;
		v[vi + 2].TexCoords[1] = UV_Y_BASE + uvModifier;
		
		v[vi + 3].Position[0] = chunk->worldPosition.x + x + 0.0f;
		v[vi + 3].Position[1] = chunk->worldPosition.y + y + 1.0f;
		v[vi + 3].Position[2] = chunk->worldPosition.z + z + 1.0f;
		v[vi + 3].TexCoords[0] = 0.0f;
		v[vi + 3].TexCoords[1] = UV_Y_BASE + uvModifier;
	}

	if (direction == eDirection::BOTTOM) {
		v[vi + 0].Position[0] = chunk->worldPosition.x + x + 0.0f;
		v[vi + 0].Position[1] = chunk->worldPosition.y + y + 0.0f;
		v[vi + 0].Position[2] = chunk->worldPosition.z + z + 0.0f;
		v[vi + 0].TexCoords[0] = 0.75f;
		v[vi + 0].TexCoords[1] = 0.0f + uvModifier;
		
		v[vi + 1].Position[0] = chunk->worldPosition.x + x + 1.0f;
		v[vi + 1].Position[1] = chunk->worldPosition.y + y + 0.0f;
		v[vi + 1].Position[2] = chunk->worldPosition.z + z + 0.0f;
		v[vi + 1].TexCoords[0] = 1.0f;
		v[vi + 1].TexCoords[1] = 0.0f + uvModifier;
		
		v[vi + 2].Position[0] = chunk->worldPosition.x + x + 1.0f;
		v[vi + 2].Position[1] = chunk->worldPosition.y + y + 0.0f;
		v[vi + 2].Position[2] = chunk->worldPosition.z + z + 1.0f;
		v[vi + 2].TexCoords[0] = 1.0f;
		v[vi + 2].TexCoords[1] = UV_Y_BASE + uvModifier;
		
		v[vi + 3].Position[0] = chunk->worldPosition.x + x + 0.0f;
		v[vi + 3].Position[1] = chunk->worldPosition.y + y + 0.0f;
		v[vi + 3].Position[2] = chunk->worldPosition.z + z + 1.0f;
		v[vi + 3].TexCoords[0] = 0.75f;
		v[vi + 3].TexCoords[1] = UV_Y_BASE + uvModifier;
	}

	if (direction == eDirection::LEFT) {
		v[vi + 0].Position[0] = chunk->worldPosition.x + x + 0.0f;
		v[vi + 0].Position[1] = chunk->worldPosition.y + y + 0.0f;
		v[vi + 0].Position[2] = chunk->worldPosition.z + z + 0.0f;
		v[vi + 0].TexCoords[0] = 0.25f;
		v[vi + 0].TexCoords[1] = 0.0f + uvModifier;
		
		v[vi + 1].Position[0] = chunk->worldPosition.x + x + 0.0f;
		v[vi + 1].Position[1] = chunk->worldPosition.y + y + 0.0f;
		v[vi + 1].Position[2] = chunk->worldPosition.z + z + 1.0f;
		v[vi + 1].TexCoords[0] = 0.5f;
		v[vi + 1].TexCoords[1] = 0.0f + uvModifier;
		
		v[vi + 2].Position[0] = chunk->worldPosition.x + x + 0.0f;
		v[vi + 2].Position[1] = chunk->worldPosition.y + y + 1.0f;
		v[vi + 2].Position[2] = chunk->worldPosition.z + z + 1.0f;
		v[vi + 2].TexCoords[0] = 0.5f;
		v[vi + 2].TexCoords[1] = UV_Y_BASE + uvModifier;
		
		v[vi + 3].Position[0] = chunk->worldPosition.x + x + 0.0f;
		v[vi + 3].Position[1] = chunk->worldPosition.y + y + 1.0f;
		v[vi + 3].Position[2] = chunk->worldPosition.z + z + 0.0f;
		v[vi + 3].TexCoords[0] = 0.25f;
		v[vi + 3].TexCoords[1] = UV_Y_BASE + uvModifier;
	}

	if (direction == eDirection::RIGHT) {
		v[vi + 0].Position[0] = chunk->worldPosition.x + x + 1.0f;
		v[vi + 0].Position[1] = chunk->worldPosition.y + y + 0.0f;
		v[vi + 0].Position[2] = chunk->worldPosition.z + z + 0.0f;
		v[vi + 0].TexCoords[0] = 0.25f;
		v[vi + 0].TexCoords[1] = 0.0f + uvModifier;
		
		v[vi + 1].Position[0] = chunk->worldPosition.x + x + 1.0f;
		v[vi + 1].Position[1] = chunk->worldPosition.y + y + 0.0f;
		v[vi + 1].Position[2] = chunk->worldPosition.z + z + 1.0f;
		v[vi + 1].TexCoords[0] = 0.5f;
		v[vi + 1].TexCoords[1] = 0.0f + uvModifier;
		
		v[vi + 2].Position[0] = chunk->worldPosition.x + x + 1.0f;
		v[vi + 2].Position[1] = chunk->worldPosition.y + y + 1.0f;
		v[vi + 2].Position[2] = chunk->worldPosition.z + z + 1.0f;
		v[vi + 2].TexCoords[0] = 0.5f;
		v[vi + 2].TexCoords[1] = UV_Y_BASE + uvModifier;
		
		v[vi + 3].Position[0] = chunk->worldPosition.x + x + 1.0f;
		v[vi + 3].Position[1] = chunk->worldPosition.y + y + 1.0f;
		v[vi + 3].Position[2] = chunk->worldPosition.z + z + 0.0f;
		v[vi + 3].TexCoords[0] = 0.25f;
		v[vi + 3].TexCoords[1] = UV_Y_BASE + uvModifier;
	}
}

