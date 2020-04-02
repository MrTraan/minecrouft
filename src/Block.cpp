#include <Block.hpp>

#include <ngLib/nglib.h>

bool BlockTypeIsCollidable( eBlockType type ) { return type != eBlockType::INACTIVE && type != eBlockType::WATER; }

void BlockGetTextureIndices( eBlockType type, float & top, float & bottom, float & side ) {
	switch ( type ) {
	case eBlockType::GRASS:
		top = 0.0f;
		bottom = 2.0f;
		side = 1.0f;
		break;
	case eBlockType::DIRT:
		top = 2.0f;
		bottom = 2.0f;
		side = 2.0f;
		break;
	case eBlockType::SNOW:
		top = 3.0f;
		bottom = 5.0f;
		side = 4.0f;
		break;
	case eBlockType::COBBLESTONE:
		top = 6.0f;
		bottom = 6.0f;
		side = 6.0f;
		break;
	case eBlockType::STONE:
		top = 7.0f;
		bottom = 7.0f;
		side = 7.0f;
		break;
	case eBlockType::SAND:
		top = 8.0f;
		bottom = 8.0f;
		side = 8.0f;
		break;
	case eBlockType::WATER:
		top = 9.0f;
		bottom = 9.0f;
		side = 9.0f;
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

