#pragma once

#include <Chunk.hpp>

#include <FastNoise/FastNoise.h>

class HeightMap {
   public:
	HeightMap() {
		fn.SetNoiseType(FastNoise::Perlin);
	}

	int GetValue(int x, int y) {
		return (int)((fn.GetValue(x, y) * CHUNK_SIZE / 2 +
		             (CHUNK_SIZE / 2)));
	}


   private:
	   FastNoise fn;
};
