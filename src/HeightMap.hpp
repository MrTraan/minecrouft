#pragma once

#include <Chunk.hpp>

#include <FastNoise/FastNoise.h>

#include <stdio.h>
#include <cstdlib>
#include <ctime>

class HeightMap {
   public:
	HeightMap() {
		std::srand(std::time(NULL));
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
