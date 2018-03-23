#pragma once

#include <Texture.hpp>

#include <map>

enum eBiome {
	GRASS,
};

class TextureManager {
   public:
	static Texture GetTexture(eBiome biome);

	static Texture LoadTexture(eBiome biome);

   private:
	static std::map<eBiome, Texture> textures;
};
