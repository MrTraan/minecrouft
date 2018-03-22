#include <glad/glad.h>

#include <stdexcept>
#include "Texture.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


Texture::Texture(const std::string& path, eImageFormat format) {
	stbi_set_flip_vertically_on_load(true);
	this->data = stbi_load(path.c_str(), &(this->Width), &(this->Height),
	                       &(this->NbrChannels), 0);
	if (!this->data) {
		printf("Could not load file %s\n", path.c_str());
		throw std::runtime_error("Failed to load texture file");
	}

	glGenTextures(1, &(this->ID));
	glBindTexture(GL_TEXTURE_2D, this->ID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->Width, this->Height, 0, format,
	             GL_UNSIGNED_BYTE, this->data);
	glGenerateMipmap(GL_TEXTURE_2D);

	// cleanup
	stbi_image_free(this->data);
}

void Texture::Bind() {
	glBindTexture(GL_TEXTURE_2D, this->ID);
}
