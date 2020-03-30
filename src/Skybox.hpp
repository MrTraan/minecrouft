#pragma once

#include <glad/glad.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <Shader.hpp>
#include <vector>

static float skyboxVertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

struct Skybox {
	void Init() {
		std::string basePath = "./resources/skybox/";
		shader.CompileFromPath( "./resources/shaders/skybox_vertex.glsl", "./resources/shaders/skybox_fragment.glsl" );
		std::vector< std::string > faces = {"right.png", "left.png", "top.png", "bottom.png", "front.png", "back.png"};
		glGenTextures( 1, &textureID );
		glBindTexture( GL_TEXTURE_CUBE_MAP, textureID );

		int width, height, nrChannels;
		for ( unsigned int i = 0; i < faces.size(); i++ ) {
			unsigned char * data = stbi_load( ( basePath + faces[ i ] ).c_str(), &width, &height, &nrChannels, 0 );
			if ( data ) {
				glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA,
				              GL_UNSIGNED_BYTE, data );
				stbi_image_free( data );
			} else {
				std::cout << "Cubemap texture failed to load at path: " << faces[ i ] << std::endl;
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

	void Draw( const glm::mat4 & view, const glm::mat4 & projection ) {
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

	Shader       shader;
	unsigned int textureID;
	unsigned int VAO, VBO;
};
