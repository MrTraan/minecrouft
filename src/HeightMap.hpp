#pragma once

#include <Chunk.hpp>

#include <FastNoise/FastNoise.h>

#include <stdio.h>
#include <cstdlib>
#include <ctime>
#include <constants.hpp>

class HeightMap {
   public:
	HeightMap() {
		std::srand((u32)std::time(0));
		int seed = std::rand();
		fn.SetSeed(seed);
		fn.SetNoiseType(FastNoise::Perlin);
	}

	u32 GetValue(int x, int y) {
		return (u32)(((float)fn.GetValue(x, y) + 1) * CHUNK_HEIGHT / 2);
	}


   private:
	FastNoise fn;
};
