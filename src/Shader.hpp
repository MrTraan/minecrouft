#pragma once

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader {
public:
	unsigned int ID;

	// Constructor reads, compiles and links subshaders
	Shader(const char* vertexPath, const char* fragmentPath);

	void Use() {
		glUseProgram(this->ID);
	}

	void SetBool(const std::string &name, bool value) const {
		glUniform1i(glGetUniformLocation(this->ID, name.c_str()), (int)value);
	}
	
	void SetInt(const std::string &name, int value) const {
		glUniform1i(glGetUniformLocation(this->ID, name.c_str()), value);
	}
	
	void SetFloat(const std::string &name, float value) const {
		glUniform1f(glGetUniformLocation(this->ID, name.c_str()), value);
	}

private:
	int checkCompileErrors(unsigned int shader, const std::string &name);
	int checkLinkErrors(unsigned int shader, const std::string &name);
};