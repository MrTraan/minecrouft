#pragma once

#include <glad/glad.h>
#include <constants.hpp>
#include <string>

typedef GLuint TextureAtlas;

TextureAtlas loadTextureAtlas(const std::string& path, s32 lines, s32 columns);
void bindTextureAtlas(TextureAtlas ta);
