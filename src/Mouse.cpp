#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <Mouse.hpp>
#include <Window.hpp>
#include <Keyboard.hpp>

#include <imgui/imgui.h>

void Mouse::Init(Window &window) {
	Mouse::glWindow = window.GetGlfwWindow();
	glfwSetInputMode(window.GetGlfwWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	Mouse::CursorDisabled = true;
}

void Mouse::Update() {
	static bool firstLoop = true;
	double xpos, ypos;

	if (Keyboard::IsKeyPressed(eKey::KEY_LEFT_SHIFT)) {
		if (Mouse::CursorDisabled) {
			glfwSetInputMode(Mouse::glWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			Mouse::CursorDisabled = false;
			Mouse::Offset = glm::vec2(0, 0);
		} else {
			glfwSetInputMode(Mouse::glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			Mouse::CursorDisabled = true;
			firstLoop = true;
		}
	}

	if (!Mouse::CursorDisabled)
		return;

	glfwGetCursorPos(Mouse::glWindow, &xpos, &ypos);

	if (firstLoop) {
		Mouse::Pos = glm::vec2(xpos, ypos);
		firstLoop = false;
	}
	glm::vec2 lastPos = Mouse::Pos;
	Mouse::Pos = glm::vec2(xpos, ypos);
	Mouse::Offset = glm::vec2(xpos - lastPos.x, ypos - lastPos.y);

	ImGui::Text("Mouse pos: %f %f", Pos.x, Pos.y);

}

glm::vec2 Mouse::Pos;
glm::vec2 Mouse::Offset;
bool Mouse::CursorDisabled = true;

GLFWwindow* Mouse::glWindow = NULL;