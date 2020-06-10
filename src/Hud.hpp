#pragma once

#include "Shader.hpp"
#include "ngLib/types.h"
#include <glm/glm.hpp>

struct Hud {
	Shader shader;

	glm::mat4x4 orthoProjection;

	u32 VAO;
	u32 VBO;

	void Init( int screenWidth, int screenHeight );
	void Draw();
	void Shutdown();
};