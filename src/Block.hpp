#pragma once

enum class eBlockType : char { INACTIVE = 0, GRASS, DIRT, SAND, STONE, COBBLESTONE, SNOW, WATER };

bool BlockTypeIsCollidable( eBlockType type );
float BlockGetResistance( eBlockType type );
void BlockGetTextureIndices( eBlockType type, float & top, float & bottom, float & side );
