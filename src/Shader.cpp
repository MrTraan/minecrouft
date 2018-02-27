#include "Shader.hpp"

static std::string readFile(const char *path) {
	std::ifstream fstream;
	std::stringstream sstream;

	fstream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	fstream.open(path);
	sstream << fstream.rdbuf();
	fstream.close();
	return sstream.str();
}

Shader::Shader(const char* vertexPath, const char* fragmentPath) {
	std::string vertexCode;
	std::string fragmentCode;

	vertexCode = readFile(vertexPath);
	fragmentCode = readFile(fragmentPath);

	unsigned int vertex, fragment;
	const char *vShaderCode;
	const char *fShaderCode;

	// Compile shaders
	vShaderCode = vertexCode.c_str();
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);
	this->checkCompileErrors(vertex, vertexPath);

	fShaderCode = fragmentCode.c_str();
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, NULL);
	glCompileShader(fragment);
	this->checkCompileErrors(fragment, fragmentPath);

	// Create shader program
	this->ID = glCreateProgram();
	glAttachShader(this->ID, vertex);
	glAttachShader(this->ID, fragment);
	glLinkProgram(this->ID);
	this->checkLinkErrors(this->ID, "Program");

	// Clean up after linking
	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

int Shader::checkCompileErrors(unsigned int shader, const std::string &name) {
	int success;
	char infoLog[1024];

	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(shader, 1024, NULL, infoLog);
		std::cerr << "Error::Shader " << name << " Compilation error: " << infoLog << std::endl;
		return (1);
	}
	return (0);
}

int Shader::checkLinkErrors(unsigned int shader, const std::string &name) {
	int success;
	char infoLog[1024];

	glGetProgramiv(shader, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shader, 1024, NULL, infoLog);
		std::cerr << "Error::Shader " << name << " Linking error: " << infoLog << std::endl;
		return (1);
	}
	return (0);
}
