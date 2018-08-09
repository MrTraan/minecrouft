#include <TextureAtlas.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <stdexcept>

TextureAtlas loadTextureAtlas(const std::string& path, s32 lines, s32 columns)
{
	int width, height, channels;

	stbi_set_flip_vertically_on_load(true);
	stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
	
	if (!data)
	{
		printf("Could not load file %s\n", path.c_str());
		throw std::runtime_error("Failed to load texture file");
	}
	
	GLenum format;
	switch (channels)
	{
	case 1: format = GL_RED; break;
	case 3: format = GL_RGB; break;
	case 4: format = GL_RGBA; break;
	}

	int singleImageWidth = width / columns;
	int singleImageHeight = height / lines;
	u8* singleImage = (u8*)malloc(sizeof(u8) * channels * singleImageHeight * singleImageWidth);
	memset(singleImage, 0, sizeof(u8) * channels * singleImageHeight * singleImageWidth);

	TextureAtlas ta;

	glGenTextures(1, &ta);
	glBindTexture(GL_TEXTURE_2D_ARRAY, ta);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, singleImageWidth, singleImageHeight, lines * columns);

	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	float aniso;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);

	for (int i = 0; i < lines; i++)
	{
		for (int j = 0; j < columns; j++)
		{
			for (int ii = 0; ii < singleImageHeight; ii++)
			{
				memcpy(
					singleImage + (ii * singleImageWidth) * sizeof(u8) * channels, // Advance ii lines
					data + ((i * singleImageHeight * width) + (j * singleImageWidth) + ii * width) * sizeof(u8) * channels,
					singleImageWidth * sizeof(u8) * channels
				);
			}
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, j + (i * columns),
				singleImageWidth, singleImageHeight, 1, format, GL_UNSIGNED_BYTE, singleImage);
		}
	}

	free(singleImage);
	stbi_image_free(data);
	return ta;
}

void bindTextureAtlas(TextureAtlas ta)
{
	glBindTexture(GL_TEXTURE_2D_ARRAY, ta);
}
