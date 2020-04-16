#include <Block.hpp>

#include <ngLib/nglib.h>

bool BlockTypeIsCollidable( eBlockType type ) { return type != eBlockType::INACTIVE && type != eBlockType::WATER; }

void BlockGetTextureIndices( eBlockType type, u8 & top, u8 & bottom, u8 & side ) {
	switch ( type ) {
	case eBlockType::GRASS:
		top = 0;
		bottom = 2;
		side = 1;
		break;
	case eBlockType::DIRT:
		top = 2;
		bottom = 2;
		side = 2;
		break;
	case eBlockType::SNOW:
		top = 3;
		bottom = 5;
		side = 4;
		break;
	case eBlockType::COBBLESTONE:
		top = 6;
		bottom = 6;
		side = 6;
		break;
	case eBlockType::STONE:
		top = 7;
		bottom = 7;
		side = 7;
		break;
	case eBlockType::SAND:
		top = 8;
		bottom = 8;
		side = 8;
		break;
	case eBlockType::WATER:
		top = 9;
		bottom = 9;
		side = 9;
		break;
	default:
		ng_assert( false );
		break;
	}
}

float BlockGetResistance( eBlockType type ) {
	switch ( type ) {
	case eBlockType::GRASS:
	case eBlockType::DIRT:
	case eBlockType::SNOW:
		return 5.0f;
	case eBlockType::COBBLESTONE:
	case eBlockType::STONE:
		return 20.0f;
	case eBlockType::SAND:
		return 3.0f;
	case eBlockType::WATER:
	case eBlockType::INACTIVE:
	default:
		ng_assert( false );
		return 0.0f;
	}
}

