#pragma once

#include <glm/glm.hpp>
#include <Shader.hpp>

struct Skybox {
	void Init();

	void Draw();

	Shader       shader;
	unsigned int textureID;
	unsigned int VAO, VBO;
};
