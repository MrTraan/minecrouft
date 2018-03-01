#pragma once

#include <glad/glad.h>
#include <stb_image.h>
#include <string>

enum eImageFormat {
	RGB = GL_RGB,
	RGBA = GL_RGBA
};

class Texture {
public:
	unsigned int ID;
	int Height;
	int Width;
	int NbrChannels;

	Texture(const std::string &path, eImageFormat format);

	void Bind();

	static void Clear() { glBindTexture(GL_TEXTURE_2D, 0); }

private:
	stbi_uc *data;
};