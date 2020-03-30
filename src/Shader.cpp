#include "Shader.hpp"
#include "ngLib/nglib.h"

static std::string readFile( const char * path ) {
	std::ifstream     fstream;
	std::stringstream sstream;

	fstream.exceptions( std::ifstream::failbit | std::ifstream::badbit );

	fstream.open( path );
	sstream << fstream.rdbuf();
	fstream.close();
	return sstream.str();
}

bool Shader::CompileFromPath( const char * vertexPath, const char * fragmentPath ) {
	std::string vertexCode = readFile( vertexPath );
	std::string fragmentCode = readFile( fragmentPath );

	return CompileFromCode( vertexCode.c_str(), fragmentCode.c_str() );
}

bool Shader::CompileFromCode( const char * vertexCode, const char * fragmentCode ) {
	// Compile shaders
	u32 vertex = glCreateShader( GL_VERTEX_SHADER );
	glShaderSource( vertex, 1, &vertexCode, NULL );
	glCompileShader( vertex );
	if ( checkCompileErrors( vertex ) != 0 ) {
		return false;
	}

	u32 fragment = glCreateShader( GL_FRAGMENT_SHADER );
	glShaderSource( fragment, 1, &fragmentCode, NULL );
	glCompileShader( fragment );
	if ( checkCompileErrors( fragment ) != 0 ) {
		return false;
	}

	// Create shader program
	ID = glCreateProgram();
	glAttachShader( ID, vertex );
	glAttachShader( ID, fragment );
	glLinkProgram( ID );
	checkLinkErrors( ID );

	// Clean up after linking
	glDeleteShader( vertex );
	glDeleteShader( fragment );

	return true;
}

int Shader::checkCompileErrors( unsigned int shader ) {
	int  success;
	char infoLog[ 1024 ];

	glGetShaderiv( shader, GL_COMPILE_STATUS, &success );
	if ( !success ) {
		glGetShaderInfoLog( shader, 1024, NULL, infoLog );
		ng::Errorf( "Shader compilation failed: %s\n", infoLog );
		return 1;
	}
	return 0;
}

int Shader::checkLinkErrors( unsigned int shader ) {
	int  success;
	char infoLog[ 1024 ];

	glGetProgramiv( shader, GL_LINK_STATUS, &success );
	if ( !success ) {
		glGetProgramInfoLog( shader, 1024, NULL, infoLog );
		ng::Errorf( "Shader link failed: %s\n", infoLog );
		return 1;
	}
	return 0;
}
