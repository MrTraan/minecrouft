#include "Shader.hpp"
#include "Game.h"
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

	return CompileFromCode( vertexCode.c_str(), vertexCode.size(), fragmentCode.c_str(), fragmentCode.size() );
}

bool Shader::CompileFromCode( const char * vertexCode, int vertexSize, const char * fragmentCode, int fragmentSize ) {
	// Compile shaders
	u32 vertex = glCreateShader( GL_VERTEX_SHADER );
	glShaderSource( vertex, 1, &vertexCode, &vertexSize );
	glCompileShader( vertex );
	if ( checkCompileErrors( vertex ) != 0 ) {
		return false;
	}

	u32 fragment = glCreateShader( GL_FRAGMENT_SHADER );
	glShaderSource( fragment, 1, &fragmentCode, &fragmentSize );
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

bool Shader::CompileFromResource( const PackerResource & vertex, const PackerResource & frag ) {
	ng_assert( vertex.type == PackerResource::Type::VERTEX_SHADER );
	ng_assert( frag.type == PackerResource::Type::FRAGMENT_SHADER );

	return CompileFromCode( ( char * )theGame->package.GrabResourceData( vertex ), vertex.size,
	                        ( char * )theGame->package.GrabResourceData( frag ), frag.size );
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
