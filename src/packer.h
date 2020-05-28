#pragma once

#include "ngLib/nglib.h"
#include <string>
#include <vector>

struct PackerResource {
	enum class Type {
		PNG,
		VERTEX_SHADER,
		FRAGMENT_SHADER,
		FONT,
		INVALID,
	};

	u64  id;
	char name[ 64 ];
	u64  offset;
	u64  size;
	Type type;
};

struct PackerPackage {
	u8 *                          data;
	u64                           size;
	std::vector< PackerResource > resourceList;

	u8 * GrabResourceData( const PackerResource & resource );
};

bool PackerReadArchive( const char * path, PackerPackage * package );
bool PackerCreateArchive( const char * resourcesPath, const char * outPath );
