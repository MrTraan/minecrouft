#include <Skybox.hpp>

#include <glad/glad.h>
#include <stb_image.h>

#include "Game.h"
#include "packer_resource_list.h"
#include "tracy/Tracy.hpp"
#include <Shader.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

static constexpr float skyboxVertices[] = {
    // positions
    -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
    1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,

    -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f,
    -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,

    1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,

    -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,

    -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,

    -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f,
    1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,
};

void Skybox::Init() {
	shader.CompileFromResource( SHADERS_SKYBOX_VERT, SHADERS_SKYBOX_FRAG );
	constexpr int            numFaces = 6;
	constexpr PackerResource faces[ numFaces ] = {
	    SKYBOX_RIGHT_PNG, SKYBOX_LEFT_PNG, SKYBOX_TOP_PNG, SKYBOX_BOTTOM_PNG, SKYBOX_FRONT_PNG, SKYBOX_BACK_PNG,
	};
	glGenTextures( 1, &textureID );
	glBindTexture( GL_TEXTURE_CUBE_MAP, textureID );

	int width, height, nrChannels;
	for ( int i = 0; i < numFaces; i++ ) {
		stbi_uc * data = stbi_load_from_memory( theGame->package.GrabResourceData( faces[ i ] ), faces[ i ].size,
		                                        &width, &height, &nrChannels, 0 );
		if ( data ) {
			glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
			              data );
			stbi_image_free( data );
		} else {
			ng::Errorf( "Cubemap texture failed to load at path: %s\n", faces[ i ] );
			stbi_image_free( data );
		}
	}
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );

	// Setup vertices
	glGenVertexArrays( 1, &VAO );
	glGenBuffers( 1, &VBO );

	glBindVertexArray( VAO );
	glBindBuffer( GL_ARRAY_BUFFER, VBO );

	glBufferData( GL_ARRAY_BUFFER, sizeof( skyboxVertices ), skyboxVertices, GL_STATIC_DRAW );

	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( float ) * 3, ( void * )0 );

	glBindVertexArray( 0 );
}

void Skybox::Draw( const glm::mat4 & view, const glm::mat4 & projection ) {
	ZoneScoped;
	glDepthMask( GL_FALSE );
	shader.Use();

	int       viewLoc = glGetUniformLocation( shader.ID, "view" );
	glm::mat4 noTranslateView = glm::mat4( glm::mat3( view ) );
	glUniformMatrix4fv( viewLoc, 1, GL_FALSE, glm::value_ptr( noTranslateView ) );

	int projLoc = glGetUniformLocation( shader.ID, "projection" );
	glUniformMatrix4fv( projLoc, 1, GL_FALSE, glm::value_ptr( projection ) );

	glBindVertexArray( this->VAO );
	glBindTexture( GL_TEXTURE_CUBE_MAP, textureID );
	glDrawArrays( GL_TRIANGLES, 0, 36 );
	glBindVertexArray( 0 );
	glDepthMask( GL_TRUE );
}
