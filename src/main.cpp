#include <glad/glad.h>

#include <LZ4.h>
#include <SDL.h>
#include <chrono>
#include <glm/glm.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_impl_sdl.h>
#include <tracy/Tracy.hpp>
#include <tracy/TracyOpenGL.hpp>

#include <Camera.hpp>
#include <Chunk.hpp>
#include <ChunkManager.hpp>
#include <Frustrum.hpp>
#include <Game.h>
#include <IO.hpp>
#include <Player.hpp>
#include <Skybox.hpp>
#include <Window.hpp>
#include <ngLib/nglib.h>
#include <Guizmo.hpp>

Game * theGame;

static void DrawDebugWindow();

int main( int ac, char ** av ) {
	( void )ac;
	( void )av;
	ng::Init();

	if ( SDL_Init( SDL_INIT_AUDIO | SDL_INIT_VIDEO ) < 0 ) {
		return 1;
	}

	theGame = new Game();

	Window &       window = theGame->window;
	Camera &       camera = theGame->camera;
	Player &       player = theGame->player;
	IO &           io = theGame->io;
	ChunkManager & chunkManager = theGame->chunkManager;
	Skybox &       skybox = theGame->skybox;
	Hud &          hud = theGame->hud;

	window.Init();

	// Setup imgui
	ImGui::CreateContext();
	ImGui_ImplSDL2_InitForOpenGL( window.glWindow, window.glContext );
	ImGui_ImplOpenGL3_Init( "#version 150" );
	Guizmo::Init();

	TracyGpuContext;

	player.position = glm::vec3( SHRT_MAX / 4, 180, SHRT_MAX / 4 );
	camera.Init( ( float )window.width / window.height, player.position, glm::vec3( 0.0f, 1.0f, 0.0f ) );
	io.Init( window );
	chunkManager.Init( player.position );
	skybox.Init();
	hud.Init();

	auto lastFrameTime = std::chrono::high_resolution_clock::now();

	while ( !window.shouldClose ) {
		ZoneScopedN( "MainLoop" );

		auto  currentFrameTime = std::chrono::high_resolution_clock::now();
		float dt = std::chrono::duration_cast< std::chrono::microseconds >( currentFrameTime - lastFrameTime ).count() /
		           1000000.0f;
		lastFrameTime = currentFrameTime;

		window.Clear();
		io.Update( window );
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame( window.glWindow );
		ImGui::NewFrame();
		Guizmo::NewFrame();

		static bool debugMouse = false;
		static bool wireframeMode = false;
		// Custom event handlers
		{
			if ( io.keyboard.IsKeyDown( KEY_ESCAPE ) ) {
				window.shouldClose = true;
			}
			if ( io.keyboard.IsKeyPressed( KEY_V ) ) {
				if ( !wireframeMode ) {
					glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
					wireframeMode = true;
				} else {
					glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
					wireframeMode = false;
				}
			}
			if ( io.keyboard.IsKeyPressed( eKey::KEY_LEFT_SHIFT ) ) {
				debugMouse = !debugMouse;
				SDL_SetRelativeMouseMode( debugMouse ? SDL_FALSE : SDL_TRUE );
				SDL_SetWindowGrab( window.glWindow, debugMouse ? SDL_FALSE : SDL_TRUE );
			}
		}

		if ( !debugMouse ) {
			camera.Update( io, player, dt );
			player.Update( io, dt );
			player.TrySelectingBlock(io, chunkManager , camera );
		}

		skybox.Draw( camera.viewMatrix, camera.projMatrix );

		chunkManager.Update( camera.position );

		chunkManager.Draw( camera );

		hud.Draw();
		ng::GetConsole().Draw();
		DrawDebugWindow();

		Guizmo::Draw( camera );

		{
			ZoneScopedN( "Render_IMGUI" );
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );
		}

		window.SwapBuffers();
		TracyGpuCollect;
		FrameMark;
	}

	chunkManager.Shutdown();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	window.Shutdown();
	ng::Shutdown();

	delete theGame;

	SDL_Quit();
	return 0;
}

void DrawDebugWindow() {
	ImGui::Begin( "Debug" );

	ImGui::Text( "Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
	             ImGui::GetIO().Framerate );

	if ( ImGui::TreeNode( "Chunk manager" ) ) {
		theGame->chunkManager.DebugDraw();
		ImGui::TreePop();
	}
	if ( ImGui::TreeNode( "Camera" ) ) {
		theGame->camera.DebugDraw();
		ImGui::TreePop();
	}
	if ( ImGui::TreeNode( "Player" ) ) {
		theGame->player.DebugDraw();
		ImGui::TreePop();
	}
	if ( ImGui::TreeNode( "IO" ) ) {
		theGame->io.DebugDraw();
		ImGui::TreePop();
	}

	ImGui::End();
}
