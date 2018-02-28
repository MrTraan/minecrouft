#pragma once

#include <glad/glad.h>
#include <glfw/glfw3.h>


class Window;

# define MAX_CONCURRENT_KEY_DOWN 8

enum eKey {
	KEY_NONE = 0,
	KEY_W = GLFW_KEY_W,
	KEY_UP = GLFW_KEY_UP,
	KEY_DOWN = GLFW_KEY_DOWN,
	KEY_LEFT = GLFW_KEY_LEFT,
	KEY_RIGHT = GLFW_KEY_RIGHT,

	KEY_ESCAPE = GLFW_KEY_ESCAPE
};

class Keyboard {
public:
	static void Init(Window &window);
	static void Update();

	static void RegisterKeyPress(int key);
	static void RegisterKeyRelease(int key);

	static bool IsKeyDown(eKey key);
	static bool IsKeyPressed(eKey key);
private:
	static int keyDowns[MAX_CONCURRENT_KEY_DOWN];
	static int keyPressed[MAX_CONCURRENT_KEY_DOWN];
};