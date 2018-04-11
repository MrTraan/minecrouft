#include <TextureManager.hpp>

Texture TextureManager::GetTexture(eBiome biome) {
	auto texture = TextureManager::textures.find(biome);

	// If texture hasnt been loaded yet, load it now
	if (texture == TextureManager::textures.end())
		return TextureManager::LoadTexture(biome);
	return texture->second;
}

Texture TextureManager::LoadTexture(eBiome biome) {
	std::string path;

	switch (biome) {
		case FOREST:
#ifdef WIN32
			path =
			    "C:\\Users\\nathan\\cpp\\minecrouft\\resources\\textures.png";
#else
			path = "../resources/textures.png";
#endif
			break;
	}

	TextureManager::textures[biome] = Texture(path, eImageFormat::RGBA);
	return (TextureManager::textures[biome]);
}

std::map<eBiome, Texture> TextureManager::textures;
