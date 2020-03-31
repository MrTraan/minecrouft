#pragma once

#include "Shader.hpp"
#include "ngLib/types.h"

struct Hud {
	Shader shader;

	u32 VAO;
	u32 VBO;

	void Init();
	void Draw();
	void Shutdown();
};