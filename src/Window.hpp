#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <exception>
#include "Keyboard.hpp"

constexpr char WINDOW_TITLE[] = "Minecrouft";
constexpr int WINDOW_WIDTH = 640;
constexpr int WINDOW_HEIGHT = 480;

static void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

class Window {
public:
	char *Title;
	int Width;
	int Height;

	Window(
		int width = WINDOW_WIDTH,
		int height = WINDOW_HEIGHT,
		char *title = (char *)WINDOW_TITLE
	) : Width(width), Height(height), Title(title) {
		if (!glfwInit()) {
			throw std::exception("Fatal Error: Could not instantiate glfw");
		}
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		
		this->glWindow = glfwCreateWindow(this->Width, this->Height, this->Title, NULL, NULL);
		if (!this->glWindow) {
			throw std::exception("Fatal Error: Could not create GLFW Window");
		}

		glfwMakeContextCurrent(this->glWindow);
		glfwSetFramebufferSizeCallback(this->glWindow, framebufferSizeCallback);

		// glad: load all OpenGL function pointers
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
			throw std::exception("Failed to initialize glad\n");


		// configure global opengl state
		glEnable(GL_DEPTH_TEST);

		// @HARDCODED
		Keyboard::Init(*this);
	}

	~Window() {
		glfwTerminate();
	}

	bool ShouldClose() {
		return glfwWindowShouldClose(this->glWindow);
	}

	void Clear() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void SwapBuffers() {
		glfwSwapBuffers(this->glWindow);
	}

	void PollEvents() {
		//@HARDCODED
		Keyboard::Update();
		glfwPollEvents();
	}

	void ProcessInput() {
		// @HARDCODED : wireframe mode hardcoded
		static bool wireframeMode = false;

		if (Keyboard::IsKeyDown(KEY_ESCAPE))
			glfwSetWindowShouldClose(this->glWindow, GL_TRUE);
		if (Keyboard::IsKeyPressed(KEY_W)) {
			if (!wireframeMode) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				wireframeMode = true;
			} else {
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				wireframeMode = false;
			}
		}
	}

	// I added a getter on this member because it should only be queried for keyboard setup
	GLFWwindow* GetGlfwWindow() {
		return this->glWindow;
	}

private:
	GLFWwindow* glWindow;
};
