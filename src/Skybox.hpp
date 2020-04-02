#pragma once

#include <glm/glm.hpp>
#include <Shader.hpp>

struct Skybox {
	void Init();

	void Draw( const glm::mat4 & view, const glm::mat4 & projection );

	Shader       shader;
	unsigned int textureID;
	unsigned int VAO, VBO;
};
