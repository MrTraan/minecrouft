#include <TextureManager.hpp>

Texture TextureManager::GetTexture(eTextureFile file) {
	auto texture = TextureManager::textures.find(file);

	// If texture hasnt been loaded yet, load it now
	if (texture == TextureManager::textures.end())
		return TextureManager::LoadTexture(file);
	return texture->second;
}

Texture TextureManager::LoadTexture(eTextureFile file) {
	std::string path;

	switch (file) {
		case BLOCKS:
#ifdef WIN32
			path =
			    "C:\\Users\\nathan\\cpp\\minecrouft\\resources\\textures.png";
#else
			path = "../resources/textures.png";
#endif
			break;
	}

	TextureManager::textures[file] = Texture(path, eImageFormat::RGBA);
	return (TextureManager::textures[file]);
}

std::map<eTextureFile, Texture> TextureManager::textures;
