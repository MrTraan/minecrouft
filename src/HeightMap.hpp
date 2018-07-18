#pragma once

#include <FastNoise/FastNoise.h>

#include <stdio.h>
#include <cstdlib>
#include <ctime>
#include <constants.hpp>

// Forward declaration
class Chunk;

class HeightMap {
   public:
	HeightMap();

	void SetupChunk(Chunk* chunk);

	FastNoise heightMapNoise;
	float surfaceFreq;
	float GetHeightAt(s32 x, s32 y);

	FastNoise elevationNoise;
	float elevationFreq;
	float elevationMultiplier;
	float GetElevationAt(s32 x, s32 y);

	FastNoise moistureNoise;
	float moistureFreq;
	float GetMoistureAt(s32 x, s32 y);

	FastNoise offsetNoise;
	float offsetFreq;
	float GetOffsetAt(s32 x, s32 y);

	FastNoise caveNoise;
	float caveFreq;
};
