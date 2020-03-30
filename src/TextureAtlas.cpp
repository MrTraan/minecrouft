#include <TextureAtlas.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "ngLib/logs.h"
#include <stb_image.h>
#include <stdexcept>

TextureAtlas loadTextureAtlas( const std::string & path, s32 lines, s32 columns ) {
	int width, height, channels;

	stbi_set_flip_vertically_on_load( true );
	stbi_uc * data = stbi_load( path.c_str(), &width, &height, &channels, 0 );

	if ( !data ) {
		printf( "Could not load file %s\n", path.c_str() );
		throw std::runtime_error( "Failed to load texture file" );
	}

	GLenum format = GL_RGBA;
	switch ( channels ) {
	case 1:
		format = GL_RED;
		break;
	case 3:
		format = GL_RGB;
		break;
	case 4:
		format = GL_RGBA;
		break;
	}

	int  singleImageWidth = width / columns;
	int  singleImageHeight = height / lines;
	u8 * singleImage = new u8[ ( size_t )channels * singleImageHeight * singleImageWidth ];

	TextureAtlas ta;

	glGenTextures( 1, &ta );
	glBindTexture( GL_TEXTURE_2D_ARRAY, ta );

	glTexParameteri( GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST );
	glTexParameteri( GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

	glTexStorage3D( GL_TEXTURE_2D_ARRAY, 3, GL_RGB8, singleImageWidth, singleImageHeight, lines * columns );

	glTexParameteri( GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT );

	for ( size_t i = 0; i < lines; i++ ) {
		for ( size_t j = 0; j < columns; j++ ) {
			for ( size_t ii = 0; ii < singleImageHeight; ii++ ) {
				memcpy( singleImage + ( ii * singleImageWidth ) * sizeof( u8 ) * channels, // Advance ii lines
				        data + ( ( i * singleImageHeight * width ) + ( j * singleImageWidth ) + ii * width ) *
				                   sizeof( u8 ) * channels,
				        singleImageWidth * sizeof( u8 ) * channels );
			}
			glTexSubImage3D( GL_TEXTURE_2D_ARRAY, 0, 0, 0, j + ( i * columns ), singleImageWidth, singleImageHeight, 1,
			                 format, GL_UNSIGNED_BYTE, singleImage );
		}
	}
	glGenerateMipmap( GL_TEXTURE_2D_ARRAY );

	float aniso;
	glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso );
	glTexParameterf( GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso );
	ng::Printf( "Aniso level: %f\n", aniso );

	delete[] singleImage;
	stbi_image_free( data );
	return ta;
}

void bindTextureAtlas( TextureAtlas ta ) { glBindTexture( GL_TEXTURE_2D_ARRAY, ta ); }
