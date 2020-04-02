#pragma once
#include "tracy/Tracy.hpp"
#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <stdexcept>

constexpr char WINDOW_TITLE[] = "Minecrouft";
constexpr int  WINDOW_WIDTH = 1080;
constexpr int  WINDOW_HEIGHT = 720;

class Window {
  public:
	int width;
	int height;

	void Init( int width = WINDOW_WIDTH, int height = WINDOW_HEIGHT, char * title = ( char * )WINDOW_TITLE ) {
		this->width = width;
		this->height = height;
#if __APPLE__
		// GL 3.2 Core + GLSL 150
		const char * glsl_version = "#version 150";
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG ); // Always required on Mac
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
#else
		// GL 3.0 + GLSL 130
		const char * glsl_version = "#version 130";
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_FLAGS, 0 );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
#endif
		SDL_WindowFlags window_flags =
		    ( SDL_WindowFlags )( SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI );
		glWindow =
		    SDL_CreateWindow( title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, window_flags );
		if ( !this->glWindow ) {
			throw std::runtime_error( "Fatal Error: Could not create GLFW Window" );
		}

		glContext = SDL_GL_CreateContext( glWindow );
		SDL_GL_MakeCurrent( glWindow, glContext );
		SDL_GL_SetSwapInterval( 1 ); // Enable vsync

		// glad: load all OpenGL function pointers
		if ( !gladLoadGL() )
			throw std::runtime_error( "Failed to initialize glad\n" );

		// configure global opengl state
		glDepthFunc( GL_LESS );
		glFrontFace( GL_CCW );

		glEnable( GL_DEPTH_TEST );

		glEnable( GL_MULTISAMPLE );
		glEnable( GL_DEBUG_OUTPUT );
		glEnable( GL_CULL_FACE );
		glCullFace( GL_BACK );

		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	}

	void Shutdown() {
		SDL_GL_DeleteContext( glContext );
		SDL_DestroyWindow( glWindow );
	}

	void Clear() {
		ZoneScoped;
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	}

	void SwapBuffers() {
		ZoneScoped;
		SDL_GL_SwapWindow( glWindow );
	}

	bool          shouldClose = false;
	SDL_Window *  glWindow;
	SDL_GLContext glContext;
};
