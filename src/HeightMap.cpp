#include <HeightMap.hpp>
#include <math.h>
#include <Chunk.hpp>


HeightMap::HeightMap() {
	int seed = std::rand();

	elevationFreq = 0.003f;
	elevationNoise.SetSeed(seed);
	elevationNoise.SetNoiseType(FastNoise::Perlin);
	elevationNoise.SetFrequency(elevationFreq);

	surfaceFreq = 0.025f;
	heightMapNoise.SetSeed(seed);
	heightMapNoise.SetNoiseType(FastNoise::Perlin);
	heightMapNoise.SetFrequency(surfaceFreq);

	caveFreq = 0.01f;
	caveNoise.SetSeed(seed);
	caveNoise.SetNoiseType(FastNoise::Perlin);
	caveNoise.SetFrequency(caveFreq);

	moistureFreq = 0.01f;
	moistureNoise.SetSeed(seed);
	moistureNoise.SetNoiseType(FastNoise::Perlin);
	moistureNoise.SetFrequency(moistureFreq);

	offsetFreq = 0.015f;
	offsetNoise.SetSeed(seed);
	offsetNoise.SetNoiseType(FastNoise::Perlin);
	offsetNoise.SetFrequency(offsetFreq);

	elevationMultiplier = 1.55f;
}

float HeightMap::GetElevationAt(s32 x, s32 y) {
	return (elevationNoise.GetNoise(x, y) + 1.0f) / 2.0f * elevationMultiplier;
}

float HeightMap::GetOffsetAt(s32 x, s32 y) {
	return (offsetNoise.GetNoise(x, y) + 1.0f) / 2.0f * CHUNK_SIZE;
}

float HeightMap::GetMoistureAt(s32 x, s32 y) {
	return (moistureNoise.GetNoise(x, y) + 1.0f) / 2.0f;
}

float HeightMap::GetHeightAt(s32 x, s32 y) {
	float exponant = GetElevationAt(x, y);
	float offset = GetOffsetAt(x, y);

	float height = (heightMapNoise.GetNoise(x, y) + 1) * CHUNK_SIZE +
		(heightMapNoise.GetNoise(x * 2, y * 2) + 1) * 0.5f * CHUNK_SIZE +
		(heightMapNoise.GetNoise(x * 4, y * 4) + 1) * 0.25f * CHUNK_SIZE;

	return pow(height, exponant) + offset;
}

void HeightMap::SetupChunk(Chunk* chunk) {
	static constexpr s32 caveLevel = (CHUNK_HEIGHT / 2) - CHUNK_SIZE;

	auto chunkPos = chunk->worldPosition;

	for (s32 x = 0; x < CHUNK_SIZE; x++) {
		for (s32 z = 0; z < CHUNK_SIZE; z++) {
			for (s32 y = 0; y < caveLevel; y++) {
						chunk->cubes[x][y][z] = eBlockType::ROCK;
						continue;
				auto height = (caveNoise.GetNoise(chunkPos.x + x, chunkPos.z + z, chunkPos.y + y));
				auto type = (caveNoise.GetNoise(chunkPos.x + x, chunkPos.z + z) + 1.f) / 2.f;
				if (height > 0.0f) {
					if (type < 0.33f) 
						chunk->cubes[x][y][z] = eBlockType::SAND;
					else if (type < 0.5f) 
						chunk->cubes[x][y][z] = eBlockType::DIRT;
					else
						chunk->cubes[x][y][z] = eBlockType::ROCK;
				}
				else
					chunk->cubes[x][y][z] = eBlockType::INACTIVE;
			}

			for (s32 y = caveLevel; y < CHUNK_HEIGHT; y++) {
				auto height = GetHeightAt(chunkPos.x + x, chunkPos.z + z) + CHUNK_HEIGHT / 2;
				if (y < height) {
					chunk->cubes[x][y][z] = eBlockType::GRASS;
				} else {
					chunk->cubes[x][y][z] = eBlockType::INACTIVE;
				}
			}
		}
	}
}
