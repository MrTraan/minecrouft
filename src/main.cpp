#include <glad/glad.h>

#include "glm/gtc/type_ptr.hpp"
#include <LZ4.h>
#include <SDL.h>
#include <algorithm>
#include <chrono>
#include <glm/glm.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_impl_sdl.h>
#include <tracy/Tracy.hpp>
#include <tracy/TracyOpenGL.hpp>

#include "packer.h"
#include <Camera.hpp>
#include <Chunk.hpp>
#include <ChunkManager.hpp>
#include <Frustrum.hpp>
#include <Game.h>
#include <Guizmo.hpp>
#include <IO.hpp>
#include <Player.hpp>
#include <Skybox.hpp>
#include <Window.hpp>
#include <ngLib/nglib.h>

#if defined( _WIN32 )
#include <filesystem>
#else
NG_UNSUPPORTED_PLATFORM // GOOD LUCK LOL
#endif

#define STB_TRUETYPE_IMPLEMENTATION // force following include to generate implementation
#include <stb_truetype.h>

Game * theGame;

static void DrawDebugWindow();

static void FixedUpdate() {
	theGame->player.FixedUpdate();
}

static void Update( float dt ) {
	theGame->camera.Update( theGame->io, theGame->player, dt );
	theGame->player.Update( theGame->io, dt );
	theGame->chunkManager.Update( theGame->player.position );
}
static void Render() {
	theGame->skybox.Draw( theGame->camera.viewMatrix, theGame->camera.projMatrix );
	theGame->chunkManager.Draw( theGame->camera );
	theGame->player.Draw( theGame->camera );
	theGame->hud.Draw();
}

int main( int ac, char ** av ) {
	ng::Init();

	if ( ac > 1 && strcmp( "--create-archive", av[ 1 ] ) == 0 ) {
		bool success = PackerCreateArchive( "C:\\Users\\natha\\code\\repos\\minecrouft\\resources", "archive.lz4" );
		ng_assert( success == true );
		return 0;
	}

	if ( SDL_Init( SDL_INIT_AUDIO | SDL_INIT_VIDEO ) < 0 ) {
		ng::Errorf( "SDL_Init failed: %s\n", SDL_GetError() );
		return 1;
	}

	// Check if save folder exists
	std::vector< std::string > worldsWithSaveFiles;
	{
		bool exists = std::filesystem::exists( saveFilesFolderPath );
		bool isDirectory = std::filesystem::is_directory( saveFilesFolderPath );

		if ( exists && !isDirectory ) {
			// What is this file doing here? Let's try to delete it
			bool success = std::filesystem::remove( saveFilesFolderPath );
			if ( !success ) {
				ng::Errorf( "Could not create save files folder, a file exists with the same name\n" );
				exit( 1 );
			}
			exists = false;
		}
		if ( !exists ) {
			bool success = std::filesystem::create_directory( saveFilesFolderPath );
			if ( !success ) {
				ng::Errorf( "Could not create save files folder\n" );
				exit( 1 );
			}
		}

		std::vector< std::string > filesInsideSaveFolder;
		ng::ListFilesInDirectory( saveFilesFolderPath, filesInsideSaveFolder );
		for ( const std::string & file : filesInsideSaveFolder ) {
			if ( file.ends_with( saveFileExtension ) ) {
				std::string fileWithoutExtension = file.substr( 0, file.size() - strlen( saveFileExtension ) );
				std::string worldName = fileWithoutExtension.substr( strlen( saveFilesFolderPath ) );
				// remove leading characters
				while ( worldName.at( 0 ) == '.' || worldName.at( 0 ) == '/' || worldName.at( 0 ) == '\\' ) {
					worldName = worldName.erase( 0, 1 );
				}
				std::string expectedMetaFilePath = fileWithoutExtension + saveFileMetaExtension;
				// Make sure it has a corresponding meta file
				auto it = std::find( filesInsideSaveFolder.begin(), filesInsideSaveFolder.end(), expectedMetaFilePath );
				bool found = it != filesInsideSaveFolder.end();
				if ( !found ) {
					ng::Errorf( "There is a save file \"%s\" with no associated meta file\n", file.c_str() );
				} else {
					worldsWithSaveFiles.push_back( worldName );
				}
			}
		}

		ng::Printf( "Found %d valid save files\n", worldsWithSaveFiles.size() );
	}

	theGame = new Game();
	theGame->state = Game::State::MENU;

	bool success = PackerReadArchive( "resources.lz4", &theGame->package );
	ng_assert( success == true );

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
	player.Init();
	camera.Init( ( float )window.width / window.height, player.position, glm::vec3( 0.0f, 1.0f, 0.0f ) );
	io.Init( window );
	skybox.Init();
	hud.Init();

	std::vector< std::string > files;
	ng::ListFilesInDirectory( ".", files );

	auto  lastFrameTime = std::chrono::high_resolution_clock::now();
	float fixedTimeStepAccumulator = 0.0f;

	while ( !window.shouldClose ) {
		ZoneScopedN( "MainLoop" );

		auto  currentFrameTime = std::chrono::high_resolution_clock::now();
		float dt = std::chrono::duration_cast< std::chrono::microseconds >( currentFrameTime - lastFrameTime ).count() /
		           1000000.0f;
		lastFrameTime = currentFrameTime;

		{
			ZoneScopedN( "NewFrameSetup" );
			window.Clear();
			io.Update( window );
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplSDL2_NewFrame( window.glWindow );
			ImGui::NewFrame();
			Guizmo::NewFrame();
		}

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

		if ( theGame->state == Game::State::PLAYING ) {
			fixedTimeStepAccumulator += dt;
			int numFixedSteps = floorf( fixedTimeStepAccumulator / FIXED_TIMESTEP );
			if ( numFixedSteps > 0 ) {
				fixedTimeStepAccumulator -= numFixedSteps * FIXED_TIMESTEP;
			}
			ImGui::Text( "Num fixed steps: %d\n", numFixedSteps );
			for ( int i = 0; i < numFixedSteps; i++ ) {
				FixedUpdate();
			}
			Update( dt );
			Render();
		}
		if ( theGame->state == Game::State::LOADING ) {
			ImGui::Begin( "loading ui mockup" );
			ImGui::Text( "Loading" );
			ImGui::End();

			chunkManager.FlushLoadingQueue();
			ChunkCoordinates playerChunk = WorldToChunkPosition( player.position );
			if ( chunkManager.ChunkIsLoaded( playerChunk ) == true ) {
				theGame->state = Game::State::PLAYING;
			}
		}
		if ( theGame->state == Game::State::MENU ) {
			ImGui::Begin( "Main menu ui mockup" );

			ImGui::Text( "Load existing world:" );
			for ( const std::string & world : worldsWithSaveFiles ) {
				if ( ImGui::Button( world.c_str() ) ) {
					// We should check if the world name is correct
					chunkManager.Init( world.c_str(), player.position );
					theGame->state = Game::State::LOADING;
				}
			}

			static char newWorldName[ 50 ] = {0};
			ImGui::Text( "Create new world:" );
			ImGui::InputText( "##world_name", newWorldName, 50 );
			if ( newWorldName[ 0 ] != 0 ) {
				ImGui::SameLine();
				if ( ImGui::Button( "Create" ) ) {
					// We should check if the world name is correct
					chunkManager.CreateNewWorld( newWorldName );
					chunkManager.Init( newWorldName, player.position );
					theGame->state = Game::State::LOADING;
				}
			}

			ImGui::End();
		}

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
