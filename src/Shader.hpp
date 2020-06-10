#pragma once

#include <GL/gl3w.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "ngLib/nglib.h"
#include "packer.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

class Shader {
  public:
	u32 ID = 0;

	bool CompileFromPath( const char * vertexPath, const char * fragmentPath );
	bool CompileFromCode( const char * vertexCode, int vertexSize, const char * fragmentCode, int fragmentSize );
	bool CompileFromResource( const PackerResourceID & vertex, const PackerResourceID & frag );

	void Use() { glUseProgram( this->ID ); }

	void SetBool( const char * name, bool value ) const {
		glUniform1i( glGetUniformLocation( this->ID, name ), ( int )value );
	}

	void SetInt( const char * name, int value ) const { glUniform1i( glGetUniformLocation( this->ID, name ), value ); }

	void SetFloat( const char * name, float value ) const {
		glUniform1f( glGetUniformLocation( this->ID, name ), value );
	}

	void SetVector( const char * name, const glm::vec3 & v ) const {
		glUniform3f( glGetUniformLocation( this->ID, name ), v.x, v.y, v.z );
	}
	
	void SetMatrix( const char * name, const glm::mat4x4 & mat ) const {
		glUniformMatrix4fv( glGetUniformLocation( ID, name ), 1, GL_FALSE, glm::value_ptr( mat ) );
	}

  private:
	int checkCompileErrors( unsigned int shader );
	int checkLinkErrors( unsigned int shader );
};