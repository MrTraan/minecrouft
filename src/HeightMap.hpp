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

	int GetValue(int x, int y) {
		return (int)((fn.GetValue(x, y) + 1) * CHUNK_SIZE / 2);
	}


   private:
	FastNoise fn;
};
