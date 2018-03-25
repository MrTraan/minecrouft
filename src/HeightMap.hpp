#pragma once

#include <Chunk.hpp>

#include <noise/noise.h>
#include <noiseutils.h>

class HeightMap {
   public:
	HeightMap() {
		heightMapBuilder.SetSourceModule(gen);
		heightMapBuilder.SetDestNoiseMap(heightMap);
		heightMapBuilder.SetDestSize(CHUNK_SIZE, CHUNK_SIZE);
	}

	void SetBounds(int x, int y) {
		heightMapBuilder.SetBounds(x / CHUNK_SIZE, x / CHUNK_SIZE + 1,
		                           y / CHUNK_SIZE, y / CHUNK_SIZE + 1);
	}

	void Build() {
		heightMapBuilder.Build();
	}

	int GetValue(int x, int y) {
		return (int)((heightMap.GetValue(x, y) + 1) * CHUNK_SIZE / 4 +
		             (CHUNK_SIZE / 2));
	}


   private:
	noise::module::Perlin gen;
	noise::utils::NoiseMap heightMap;
	noise::utils::NoiseMapBuilderPlane heightMapBuilder;
};
