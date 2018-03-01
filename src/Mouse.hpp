#pragma once

#include <glm/glm.hpp>

#include <Window.hpp>

class Mouse {
public:
	static void Init(Window &window);
	static void Update();

	static glm::vec2 Pos;
	static glm::vec2 Offset;

	static bool CursorDisabled;

private:
	static GLFWwindow *glWindow;
};
