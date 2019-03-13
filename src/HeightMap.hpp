#pragma once

#include <FastNoise/FastNoise.h>

#include <stdio.h>
#include <cstdlib>
#include <ctime>
#include <constants.hpp>

// Forward declaration
struct Chunk;

class HeightMap {
public:
	HeightMap();

	void SetupChunk(Chunk* chunk);
	void GenerateWorld();

private:
	u8* world;
	FastNoise heightMapNoise;
	float surfaceFreq;
	float GetHeightAt(s32 x, s32 y);

	FastNoise elevationNoise;
	float elevationFreq;
	float elevationMultiplier;

	FastNoise moistureNoise;
	float moistureFreq;

	FastNoise offsetNoise;
	float offsetFreq;

	FastNoise caveNoise;
	float caveFreq;

	inline float GetElevationAt(s32 x, s32 y) {
		return (elevationNoise.GetNoise((float)x, (float)y) + 1.0f) / 2.0f * elevationMultiplier;
	}

	inline float GetOffsetAt(s32 x, s32 y) {
		return (offsetNoise.GetNoise((float)x, (float)y) + 1.0f) / 2.0f * CHUNK_SIZE;
	}

	inline float GetMoistureAt(s32 x, s32 y) {
		return (moistureNoise.GetNoise((float)x, (float)y) + 1.0f) / 2.0f;
	}

};
