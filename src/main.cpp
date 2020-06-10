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

const unsigned int SHADOW_SIZE = 4096;
unsigned int       depthMap;
unsigned int       colorMap;
unsigned int       depthMapFBO;
unsigned int       colorMapFBO;
unsigned int       fakeTextureMap;

static void FixedUpdate() { theGame->player.FixedUpdate(); }

static void Update( float dt ) {
	theGame->camera.Update( theGame->io, theGame->player, dt );
	theGame->player.Update( theGame->io, dt );
	theGame->chunkManager.Update( theGame->player.position );
}

static void Render() {
	glViewport( 0, 0, SHADOW_SIZE, SHADOW_SIZE );
	glBindFramebuffer( GL_FRAMEBUFFER, depthMapFBO );
	glClear( GL_DEPTH_BUFFER_BIT );
	glEnable( GL_DEPTH_TEST );
	glDisable( GL_BLEND );
	glCullFace( GL_NONE );
	theGame->chunkManager.DrawShadows( theGame->camera.frustrum );
	glCullFace( GL_BACK );
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	glViewport( 0, 0, theGame->window.width, theGame->window.height );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable( GL_BLEND );

	Camera & cam = theGame->camera;
	{

		glm::mat4 viewProj = cam.projMatrix * cam.viewMatrix;
		glm::mat4 lightViewProj = cam.lightProjectionMatrix * cam.lightViewMatrix;
		glm::mat4 matrices[ 4 ] = {
		    cam.lightProjectionMatrix,
		    cam.lightViewMatrix,
		    lightViewProj,
		    lightViewProj,
		};
		glm::vec4 vectors[ 2 ] = {
		    glm::vec4( cam.lightPosition, 1.0f ),
		    glm::vec4( cam.lightDirection, 1.0f ),
		};
		glBindBuffer( GL_UNIFORM_BUFFER, cam.uboMatrices );
		glBufferSubData( GL_UNIFORM_BUFFER, 0, sizeof( glm::mat4 ) * 4, matrices );
		glBufferSubData( GL_UNIFORM_BUFFER, sizeof( glm::mat4 ) * 4, sizeof( glm::vec4 ) * 2, vectors );
		glBindBuffer( GL_UNIFORM_BUFFER, 0 );
	}

	glViewport( 0, 0, SHADOW_SIZE, SHADOW_SIZE );
	glBindFramebuffer( GL_FRAMEBUFFER, colorMapFBO );
	glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
	glEnable( GL_DEPTH_TEST );
	glDisable( GL_BLEND );
	theGame->chunkManager.Draw( theGame->camera.frustrum, fakeTextureMap );
	Guizmo::Draw();
	Guizmo::NewFrame();
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	glViewport( 0, 0, theGame->window.width, theGame->window.height );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable( GL_BLEND );

	{
		glm::mat4 viewProj = cam.projMatrix * cam.viewMatrix;
		glm::mat4 lightViewProj = cam.lightProjectionMatrix * cam.lightViewMatrix;
		glm::mat4 matrices[ 4 ] = {
		    cam.projMatrix,
		    cam.viewMatrix,
		    viewProj,
		    lightViewProj,
		};
		glm::vec4 vectors[ 2 ] = {
		    glm::vec4( cam.lightPosition, 1.0f ),
		    glm::vec4( cam.lightDirection, 1.0f ),
		};
		glBindBuffer( GL_UNIFORM_BUFFER, cam.uboMatrices );
		glBufferSubData( GL_UNIFORM_BUFFER, 0, sizeof( glm::mat4 ) * 4, matrices );
		glBufferSubData( GL_UNIFORM_BUFFER, sizeof( glm::mat4 ) * 4, sizeof( glm::vec4 ) * 2, vectors );
		glBindBuffer( GL_UNIFORM_BUFFER, 0 );
	}

	ImGui::Image( ( ImTextureID )depthMap, ImVec2( 512, 512 ), ImVec2(0, 1), ImVec2(1, 0) );
	ImGui::Image( ( ImTextureID )colorMap, ImVec2( 512, 512 ), ImVec2(0, 1), ImVec2(1, 0) );

	theGame->skybox.Draw();
	theGame->chunkManager.Draw( theGame->camera.frustrum, depthMap );
	theGame->player.Draw();
	theGame->hud.Draw();
}

int main( int ac, char ** av ) {
	ng::Init();

	if ( ac > 1 && strcmp( "--create-archive", av[ 1 ] ) == 0 ) {
		bool success = PackerCreateArchive( "W:\\minecrouft\\resources", "resources.lz4" );
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
			if ( ng::IsDirectory( file.c_str() ) ) {
				std::string worldName = std::filesystem::path( file ).filename().string();
				worldsWithSaveFiles.push_back( worldName );
			}
		}

		ng::Printf( "Found %d valid save files\n", worldsWithSaveFiles.size() );
	}

	theGame = new Game();
	theGame->state = Game::State::MENU;

	if ( true ) {
		bool success = PackerCreateRuntimeArchive( FS_BASE_PATH, &theGame->package );
		ng_assert( success == true );
	} else {
		bool success = PackerReadArchive( "resources.lz4", &theGame->package );
		ng_assert( success == true );
	}

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
	ImGuiIO &              imio = ImGui::GetIO();
	constexpr const char * imguiSaveFilePath = "C:\\Users\\natha\\Desktop\\minecrouft_imgui.ini";
	imio.IniFilename = imguiSaveFilePath;
	imio.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
	imio.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	ImGuiStyle & style = ImGui::GetStyle();
	if ( imio.ConfigFlags & ImGuiConfigFlags_ViewportsEnable ) {
		style.WindowRounding = 0.0f;
		style.Colors[ ImGuiCol_WindowBg ].w = 1.0f;
	}
	ImGui_ImplSDL2_InitForOpenGL( window.glWindow, window.glContext );
	ImGui_ImplOpenGL3_Init( "#version 150" );
	Guizmo::Init();

	TracyGpuContext;

	player.position = glm::vec3( SHRT_MAX / 4, 180, SHRT_MAX / 4 );
	player.Init();
	camera.Init( ( float )window.width / window.height, player.position, glm::vec3( 0.0f, 1.0f, 0.0f ) );
	io.Init( window );
	skybox.Init();
	hud.Init( window.width, window.height );

	// glGenFramebuffers( 1, &depthMapFBO );
	// glGenTextures( 1, &depthMap );
	// glBindTexture( GL_TEXTURE_2D, depthMap );
	// glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_SIZE, SHADOW_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT,
	//              NULL );
	// glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	// glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	// glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
	// glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
	// float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	// glTexParameterfv( GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor );
	// glBindFramebuffer( GL_FRAMEBUFFER, depthMapFBO );
	// glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0 );
	// glDrawBuffer( GL_NONE );
	// glReadBuffer( GL_NONE );
	// glBindFramebuffer( GL_FRAMEBUFFER, 0 );

	{
		GLenum format = GL_RGBA;
		glGenTextures( 1, &fakeTextureMap );
		glBindTexture( GL_TEXTURE_2D, fakeTextureMap );

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

		u32 * fakeTextureData = new u32[ 8 * 8 ];
		memset( fakeTextureData, 0, 8 * 8 * sizeof( u32 ) );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 8, 8, 0, format, GL_UNSIGNED_BYTE, fakeTextureData );
		delete[] fakeTextureData;
	}

	glGenFramebuffers( 1, &depthMapFBO );
	glGenFramebuffers( 1, &colorMapFBO );

	glGenTextures( 1, &depthMap );
	glBindTexture( GL_TEXTURE_2D, depthMap );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_SIZE, SHADOW_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT,
	              NULL );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv( GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor );

	glBindFramebuffer( GL_FRAMEBUFFER, depthMapFBO );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0 );
	glDrawBuffer( GL_NONE );
	glReadBuffer( GL_NONE );
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );

	glGenTextures( 1, &colorMap );
	glBindTexture( GL_TEXTURE_2D, colorMap );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, SHADOW_SIZE, SHADOW_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glBindFramebuffer( GL_FRAMEBUFFER, colorMapFBO );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorMap, 0 );
	unsigned int rbo;
	glGenRenderbuffers( 1, &rbo );
	glBindRenderbuffer( GL_RENDERBUFFER, rbo );
	glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SHADOW_SIZE, SHADOW_SIZE );
	glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo );
	if ( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
		ng::Errorf( "ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n" );
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	glBindRenderbuffer( GL_RENDERBUFFER, 0 );

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
				io.mouse.debugMouse = !io.mouse.debugMouse;
				SDL_SetRelativeMouseMode( io.mouse.debugMouse ? SDL_FALSE : SDL_TRUE );
				SDL_SetWindowGrab( window.glWindow, io.mouse.debugMouse ? SDL_FALSE : SDL_TRUE );
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

			static char newWorldName[ 50 ] = { 0 };
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

		Guizmo::Draw();
		{
			ZoneScopedN( "Render_IMGUI" );
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );
			if ( imio.ConfigFlags & ImGuiConfigFlags_ViewportsEnable ) {
				SDL_Window *  backup_current_window = SDL_GL_GetCurrentWindow();
				SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
				SDL_GL_MakeCurrent( backup_current_window, backup_current_context );
			}
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
	//bool demoOpen = true;
	//ImGui::ShowDemoWindow( &demoOpen );

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
