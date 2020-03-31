#pragma once
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <stdexcept>
#include "tracy/Tracy.hpp"

constexpr char WINDOW_TITLE[] = "Minecrouft";
constexpr int  WINDOW_WIDTH = 1080;
constexpr int  WINDOW_HEIGHT = 720;

static void framebufferSizeCallback( GLFWwindow * window, int width, int height ) { glViewport( 0, 0, width, height ); }

static void glfwErrorCallback( int code, const char * msg ) { printf( "Glfw error: %s\n", msg ); }

class Window {
  public:
	int width;
	int height;

	void Init( int width = WINDOW_WIDTH, int height = WINDOW_HEIGHT, char * title = ( char * )WINDOW_TITLE ) {
		this->width = width;
		this->height = height;
		if ( !glfwInit() ) {
			throw std::runtime_error( "Fatal Error: Could not instantiate glfw" );
		}
		glfwSetErrorCallback( glfwErrorCallback );
		glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
		glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
		glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
		glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
		//		glfwWindowHint(GLFW_SAMPLES, 4);

		auto monitor = glfwGetPrimaryMonitor();
		auto videoMode = glfwGetVideoMode( monitor );
		this->glWindow = glfwCreateWindow( width, height, title, NULL, NULL );
		///* this->glWindow = glfwCreateWindow(2048, 1152, */
		// this->glWindow = glfwCreateWindow(videoMode->width, videoMode->height,
		//                            this->Title, monitor, NULL);
		if ( !this->glWindow ) {
			throw std::runtime_error( "Fatal Error: Could not create GLFW Window" );
		}

		glfwMakeContextCurrent( this->glWindow );
		glfwSetFramebufferSizeCallback( this->glWindow, framebufferSizeCallback );
		glfwSwapInterval( 1 );

		// glad: load all OpenGL function pointers
		if ( !gladLoadGLLoader( ( GLADloadproc )glfwGetProcAddress ) )
			throw std::runtime_error( "Failed to initialize glad\n" );

		// configure global opengl state
		glEnable( GL_DEPTH_TEST );
	}

	void Shutdown() {
		glfwDestroyWindow( glWindow );
		glfwTerminate();
	}

	bool ShouldClose() { return glfwWindowShouldClose( this->glWindow ) != 0; }

	void Clear() { glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); }

	void SwapBuffers() {
		ZoneScoped;
		glfwSwapBuffers( this->glWindow );
	}

	void PollEvents() { glfwPollEvents(); }

	GLFWwindow * GetGlfwWindow() { return this->glWindow; }

  private:
	GLFWwindow * glWindow;
};
