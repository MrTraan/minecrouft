#pragma once
#include <GLFW/glfw3.h>
#include <exception>

constexpr char WINDOW_TITLE[] = "Minecrouft";
constexpr int WINDOW_WIDTH = 640;
constexpr int WINDOW_HEIGHT = 480;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	static bool wireframeMode = false;
	if (key == GLFW_KEY_W && action == GLFW_PRESS) {
			if (!wireframeMode) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				wireframeMode = true;
			} else {
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				wireframeMode = false;
			}
	}
}

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

		// Hacky callback for tests
		glfwSetKeyCallback(this->glWindow, key_callback);
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
		glfwPollEvents();
	}

	void ProcessInput() {
		if (glfwGetKey(this->glWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(this->glWindow, GL_TRUE);
	}

private:
	GLFWwindow *glWindow;
};
