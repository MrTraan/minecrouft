#pragma once

#include <Texture.hpp>

#include <map>

enum eTextureFile {
	BLOCKS,
};

class TextureManager {
   public:
	static Texture GetTexture(eTextureFile biome);

	static Texture LoadTexture(eTextureFile biome);

   private:
	static std::map<eTextureFile, Texture> textures;
};
