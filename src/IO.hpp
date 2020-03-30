#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Window;

enum eKey {
	KEY_NONE = 0,
	KEY_W = GLFW_KEY_W,
	KEY_A = GLFW_KEY_A,
	KEY_S = GLFW_KEY_S,
	KEY_D = GLFW_KEY_D,
	KEY_V = GLFW_KEY_V,

	KEY_UP = GLFW_KEY_UP,
	KEY_DOWN = GLFW_KEY_DOWN,
	KEY_LEFT = GLFW_KEY_LEFT,
	KEY_RIGHT = GLFW_KEY_RIGHT,

	KEY_LEFT_SHIFT = GLFW_KEY_LEFT_SHIFT,
	KEY_ESCAPE = GLFW_KEY_ESCAPE,
	KEY_SPACE = GLFW_KEY_SPACE
};

constexpr int MAX_CONCURRENT_KEY_DOWN = 16;

struct Keyboard {
	void Init( Window & window );
	void Update();

	void RegisterKeyPress( int key );
	void RegisterKeyRelease( int key );

	bool IsKeyDown( eKey key ) const;
	bool IsKeyPressed( eKey key ) const;

	// A key is down until it is released
	int keyDowns[ MAX_CONCURRENT_KEY_DOWN ];
	// A key was pressed this frame
	int keyPressed[ MAX_CONCURRENT_KEY_DOWN ];
};

struct Mouse {
	void Init( Window & window );
	void Update( Window & window );

	glm::vec2 pos;
	glm::vec2 offset;

  private:
	GLFWwindow * glWindow;
};

struct IO {
	Keyboard keyboard;
	Mouse    mouse;

	void Init( Window & window ) {
		keyboard.Init( window );
		mouse.Init( window );
	}

	void Update( Window & window ) {
		keyboard.Update();
		mouse.Update( window );
	}
};

