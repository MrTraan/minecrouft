#include <IO.hpp>

#include <Window.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw_gl3.h>
#include <vector>

static std::vector< int > keyPressEvents;
static std::vector< int > keyReleaseEvents;

void key_callback( GLFWwindow * window, int key, int scancode, int action, int mods ) {
	if ( action == GLFW_PRESS ) {
		keyPressEvents.push_back( key );
	} else if ( action == GLFW_RELEASE ) {
		keyReleaseEvents.push_back( key );
	}

	ImGui_ImplGlfw_KeyCallback( window, key, scancode, action, mods );
}

void Keyboard::Init( Window & window ) {
	GLFWwindow * glWindow = window.GetGlfwWindow();

	glfwSetKeyCallback( glWindow, key_callback );
	glfwSetCharCallback( glWindow, ImGui_ImplGlfw_CharCallback );

	for ( int i = 0; i < MAX_CONCURRENT_KEY_DOWN; i++ ) {
		keyDowns[ i ] = KEY_NONE;
		keyPressed[ i ] = KEY_NONE;
	}
}

void Keyboard::Update() {
	for ( int & k : keyPressed ) {
		k = KEY_NONE;
	}

	for ( int event : keyReleaseEvents ) {
		for ( int & keyDown : keyDowns ) {
			if ( keyDown == event ) {
				keyDown = KEY_NONE;
			}
		}
	}

	for ( int i = 0; i < MAX_CONCURRENT_KEY_DOWN && i < keyPressEvents.size(); i++ ) {
		int key = keyPressEvents[ i ];
		for ( int & keyDown : keyDowns ) {
			if ( keyDown == KEY_NONE ) {
				keyDown = key;
				break;
			}
		}

		for ( int & keyPress : keyPressed ) {
			if ( keyPress == KEY_NONE ) {
				keyPress = key;
				break;
			}
		}
	}

	keyPressEvents.clear();
	keyReleaseEvents.clear();
}

bool Keyboard::IsKeyDown( eKey key ) const {
	for ( int keyDown : Keyboard::keyDowns ) {
		if ( keyDown == key ) {
			return true;
		}
	}
	return false;
}

bool Keyboard::IsKeyPressed( eKey key ) const {
	for ( int i : Keyboard::keyPressed ) {
		if ( i == key ) {
			return true;
		}
	}
	return false;
}

void Mouse::Init( Window & window ) {
	double xpos, ypos;
	glfwGetCursorPos( window.GetGlfwWindow(), &xpos, &ypos );
	glfwSetInputMode( window.GetGlfwWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED );
	pos = glm::vec2( xpos, ypos );
}

void Mouse::Update( Window & window ) {
	double xpos, ypos;
	glfwGetCursorPos( window.GetGlfwWindow(), &xpos, &ypos );
	glm::vec2 lastPos = pos;
	pos = glm::vec2( xpos, ypos );
	offset = pos - lastPos;
}
