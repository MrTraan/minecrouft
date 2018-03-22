#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include "Keyboard.hpp"
#include "Window.hpp"

void key_callback(GLFWwindow* window,
                  int key,
                  int scancode,
                  int action,
                  int mods) {
	if (action == GLFW_PRESS)
		Keyboard::RegisterKeyPress(key);
	else if (action == GLFW_RELEASE)
		Keyboard::RegisterKeyRelease(key);
}

void Keyboard::Init(Window& window) {
	GLFWwindow* glWindow = window.GetGlfwWindow();

	glfwSetKeyCallback(glWindow, key_callback);

	for (int i = 0; i < MAX_CONCURRENT_KEY_DOWN; i++) {
		Keyboard::keyDowns[i] = KEY_NONE;
		Keyboard::keyPressed[i] = KEY_NONE;
	}
}

// Keys will be registered through callback,
// So we just want to flush key state
void Keyboard::Update() {
	for (int i = 0; i < MAX_CONCURRENT_KEY_DOWN; i++)
		Keyboard::keyPressed[i] = KEY_NONE;
}

void Keyboard::RegisterKeyPress(int key) {
	for (int i = 0; i < MAX_CONCURRENT_KEY_DOWN; i++) {
		if (Keyboard::keyDowns[i] == KEY_NONE) {
			Keyboard::keyDowns[i] = key;
			break;
		}
	}

	for (int i = 0; i < MAX_CONCURRENT_KEY_DOWN; i++) {
		if (Keyboard::keyPressed[i] == KEY_NONE) {
			Keyboard::keyPressed[i] = key;
			break;
		}
	}
}

void Keyboard::RegisterKeyRelease(int key) {
	for (int i = 0; i < MAX_CONCURRENT_KEY_DOWN; i++) {
		if (Keyboard::keyDowns[i] == key) {
			// We could break here but at least we make sure to clean if a key
			// has been registered as pressed twice, and it doesnt cost much
			Keyboard::keyDowns[i] = KEY_NONE;
		}
	}
}

bool Keyboard::IsKeyDown(eKey key) {
	for (int i = 0; i < MAX_CONCURRENT_KEY_DOWN; i++) {
		if (Keyboard::keyDowns[i] == key) {
			return true;
		}
	}
	return false;
}

// This is only true for the frame the key has been pressed
bool Keyboard::IsKeyPressed(eKey key) {
	for (int i = 0; i < MAX_CONCURRENT_KEY_DOWN; i++) {
		if (Keyboard::keyPressed[i] == key) {
			return true;
		}
	}
	return false;
}

int Keyboard::keyDowns[MAX_CONCURRENT_KEY_DOWN] = {};
int Keyboard::keyPressed[MAX_CONCURRENT_KEY_DOWN] = {};
