#pragma once

#include "packer.h"
#include <constants.hpp>
#include <GL/gl3w.h>
#include <string>

typedef GLuint TextureAtlas;

TextureAtlas loadTextureAtlas( const PackerResourceID & resource, s32 lines, s32 columns );
void         bindTextureAtlas( TextureAtlas ta );
