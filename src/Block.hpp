#pragma once
#include "constants.hpp"

enum class eBlockType : char { INACTIVE = 0, GRASS, DIRT, SAND, STONE, COBBLESTONE, SNOW, WATER, WOOD, LEAVES };

bool BlockTypeIsCollidable( eBlockType type );
float BlockGetResistance( eBlockType type );
void BlockGetTextureIndices( eBlockType type, u8 & top, u8 & bottom, u8 & side );
