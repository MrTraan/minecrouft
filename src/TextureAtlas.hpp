#pragma once

#include "packer.h"
#include <constants.hpp>
#include <glad/glad.h>
#include <string>

typedef GLuint TextureAtlas;

TextureAtlas loadTextureAtlas( const PackerResource & resource, s32 lines, s32 columns );
void         bindTextureAtlas( TextureAtlas ta );
