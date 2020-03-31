#include <IO.hpp>

#include <Window.hpp>

#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_impl_sdl.h>
#include <vector>
#include "Game.h"

void IO::Update( Window & window ) {
	keyboard.Update( window );
	mouse.Update( window );

	SDL_Event event;
	while ( SDL_PollEvent( &event ) ) {
		ImGui_ImplSDL2_ProcessEvent( &event );
		if ( event.type == SDL_QUIT ) {
			window.shouldClose = true;
		}
		if ( event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
		     event.window.windowID == SDL_GetWindowID( window.glWindow ) ) {
			window.shouldClose = true;
		}
		if ( event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED ) {
			window.width = event.window.data1;
			window.height = event.window.data2;
			glViewport( 0, 0, window.width, window.height );
			theGame->camera.UpdateProjectionMatrix( (float)window.width / window.height );
			// TODO: Refresh UI size
		}
		if ( event.type == SDL_KEYDOWN ) {
			auto key = event.key.keysym.sym;
			keyboard.RegisterKeyDown( key );
		}
		if ( event.type == SDL_KEYUP ) {
			auto key = event.key.keysym.sym;
			keyboard.RegisterKeyRelease( key );
		}
		if ( event.type == SDL_MOUSEMOTION ) {
			mouse.offset = glm::vec2( event.motion.xrel, event.motion.yrel );
		}
	}
}

void IO::DebugDraw() {
	std::string keyDowns;
	for ( int & keyDown : keyboard.keyDowns ) {
		if ( keyDown == KEY_SPACE )
			keyDowns += "P ";
		else if ( keyDown == KEY_W )
			keyDowns += "W ";
		else if ( keyDown == KEY_A )
			keyDowns += "A ";
		else if ( keyDown == KEY_S )
			keyDowns += "S ";
		else if ( keyDown == KEY_D )
			keyDowns += "D ";
		else if ( keyDown == KEY_NONE )
			keyDowns += "X ";
		else 
			keyDowns += "O ";
	}

	ImGui::Text("Key downs:\n%s", keyDowns.c_str() );
}

void Keyboard::Init( Window & window ) {
	( void )window;
	for ( int i = 0; i < MAX_CONCURRENT_KEY_DOWN; i++ ) {
		keyDowns[ i ] = KEY_NONE;
		keyPressed[ i ] = KEY_NONE;
	}
}

void Keyboard::Update( Window & window ) {
	for ( int & k : keyPressed ) {
		k = KEY_NONE;
	}
}

void Keyboard::RegisterKeyDown( SDL_Keycode key ) {
	for ( int & keyPress : keyPressed ) {
		if ( keyPress == KEY_NONE ) {
			keyPress = key;
			break;
		}
	}
	for ( int & keyDown : keyDowns ) {
		// Check if key is already down
		if ( keyDown == key ) {
			return;
		}
	}
	for ( int & keyDown : keyDowns ) {
		if ( keyDown == KEY_NONE ) {
			keyDown = key;
			break;
		}
	}
}

void Keyboard::RegisterKeyRelease( SDL_Keycode key ) {
	for ( int & keyDown : keyDowns ) {
		if ( keyDown == key ) {
			keyDown = KEY_NONE;
		}
	}
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
	SDL_SetRelativeMouseMode( SDL_TRUE );
	SDL_SetWindowGrab( window.glWindow, SDL_TRUE );
}

void Mouse::Update( Window & window ) {
	offset.x = 0;
	offset.y = 0;
}
