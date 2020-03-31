#pragma once
#include <glad/glad.h>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>

class Window;

enum eKey {
	KEY_NONE = 0,
	KEY_W = SDLK_w,
	KEY_A = SDLK_a,
	KEY_S = SDLK_s,
	KEY_D = SDLK_d,
	KEY_V = SDLK_v,

	KEY_UP = SDLK_UP,
	KEY_DOWN = SDLK_DOWN,
	KEY_LEFT = SDLK_LEFT,
	KEY_RIGHT = SDLK_RIGHT,

	KEY_LEFT_SHIFT = SDLK_LSHIFT,
	KEY_RIGHT_SHIFT = SDLK_RSHIFT,
	KEY_ESCAPE = SDLK_ESCAPE,
	KEY_SPACE = SDLK_SPACE,
	KEY_ENTER = SDLK_RETURN,
};

constexpr int MAX_CONCURRENT_KEY_DOWN = 16;

struct Keyboard {
	void Init( Window & window );
	void Update( Window & window );

	void RegisterKeyDown( SDL_Keycode key );
	void RegisterKeyRelease( SDL_Keycode key );

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

	glm::vec2 offset;
};

struct IO {
	Keyboard keyboard;
	Mouse    mouse;

	void Init( Window & window ) {
		keyboard.Init( window );
		mouse.Init( window );
	}

	void Update( Window & window );

	void DebugDraw();
};

