#pragma once

#include <glad/glad.h>

#include "ngLib/nglib.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

class Shader {
  public:
	u32 ID = 0;

	bool CompileFromPath( const char * vertexPath, const char * fragmentPath );
	bool CompileFromCode( const char * vertexCode, const char * fragmentCode );

	void Use() { glUseProgram( this->ID ); }

	void SetBool( const char * name, bool value ) const {
		glUniform1i( glGetUniformLocation( this->ID, name ), ( int )value );
	}

	void SetInt( const char * name, int value ) const { glUniform1i( glGetUniformLocation( this->ID, name ), value ); }

	void SetFloat( const char * name, float value ) const {
		glUniform1f( glGetUniformLocation( this->ID, name ), value );
	}

  private:
	int checkCompileErrors( unsigned int shader );
	int checkLinkErrors( unsigned int shader );
};