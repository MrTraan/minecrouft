#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <LZ4.h>
#include <glm/glm.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw_gl3.h>
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

Game * theGame;

static void DrawDebugWindow();

void GLAPIENTRY MessageCallback( GLenum         source,
                                 GLenum         type,
                                 GLuint         id,
                                 GLenum         severity,
                                 GLsizei        length,
                                 const GLchar * message,
                                 const void *   userParam ) {
	if ( severity == GL_DEBUG_SEVERITY_NOTIFICATION )
		return;
	fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
	         ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ), type, severity, message );
}

int main( void ) {
	ng::Init();

	theGame = new Game();

	Window         window;
	Camera &       camera = theGame->camera;
	Player &       player = theGame->player;
	IO &           io = theGame->io;
	ChunkManager & chunkManager = theGame->chunkManager;
	Skybox &       skybox = theGame->skybox;

	window.Init();

	// Setup imgui
	ImGui::CreateContext();
	ImGuiIO & imio = ImGui::GetIO();
	( void )imio;
	ImGui_ImplGlfwGL3_Init( window.GetGlfwWindow(), false );
	TracyGpuContext;

	player.position = glm::vec3( SHRT_MAX / 4, 180, SHRT_MAX / 4 );
	camera.Init( ( float )window.width / window.height, player.position, glm::vec3( 0.0f, 1.0f, 0.0f ) );
	io.Init( window );
	chunkManager.Init( player.position );
	skybox.Init();

	float dt = 0.0f;
	float lastFrame = 0.0f;

	glEnable( GL_MULTISAMPLE );
	glEnable( GL_DEBUG_OUTPUT );
	glDebugMessageCallback( MessageCallback, 0 );
	glEnable( GL_CULL_FACE );
	glCullFace( GL_BACK );

	auto playerPosition = camera.position;
	auto playerChunkPosition = WorldToChunkPosition( playerPosition );

	auto playerChunk = chunkManager.chunks.at( playerChunkPosition );
	ng_assert( playerChunk != nullptr );

	auto inputSize = sizeof( playerChunk->cubes );
	ng::Printf( "Size before compression: %llu\n", inputSize );
	int    sizeCompressed;
	char * compressedData;
	auto   minSize = LZ4_compressBound( inputSize );
	compressedData = new char[ minSize ];
	{
		ZoneScopedN( "One Chunk compression" );
		sizeCompressed =
		    LZ4_compress_default( ( char * )( &( playerChunk->cubes ) ), compressedData, inputSize, minSize );
	}
	{
		Chunk output;
		ZoneScopedN( "One Chunk decompression" );
		auto ret = LZ4_decompress_safe( compressedData, ( char * )( &( output.cubes ) ), sizeCompressed, inputSize );
		ng_assert( ret > 0 );
	}
	delete[] compressedData;
	ng::Printf( "Size after compression: %llu\n", sizeCompressed );

	while ( !window.ShouldClose() ) {
		ZoneScopedN( "MainLoop" );

		float currentFrame = ( float )glfwGetTime();
		dt = currentFrame - lastFrame;
		lastFrame = currentFrame;

		window.Clear();
		window.PollEvents();
		ImGui_ImplGlfwGL3_NewFrame();

		io.Update( window );

		static bool debugMouse = false;
		static bool wireframeMode = false;
		// Custom event handlers
		{
			if ( io.keyboard.IsKeyDown( KEY_ESCAPE ) ) {
				glfwSetWindowShouldClose( window.GetGlfwWindow(), GL_TRUE );
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
				glfwSetInputMode( window.GetGlfwWindow(), GLFW_CURSOR,
				                  debugMouse ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED );
			}
		}

		if ( !debugMouse ) {
			camera.Update( io, player, dt );
			player.Update( io, dt );
		}

		skybox.Draw( camera.viewMatrix, camera.projMatrix );

		chunkManager.Update( camera.position );

		chunkManager.Draw( camera );

		ng::GetConsole().Draw();
		DrawDebugWindow();

		{
			ZoneScopedN( "Render_IMGUI" );
			ImGui::Render();
			ImGui_ImplGlfwGL3_RenderDrawData( ImGui::GetDrawData() );
		}

		window.SwapBuffers();
		TracyGpuCollect;
		FrameMark;
	}

	chunkManager.Shutdown();

	ImGui_ImplGlfwGL3_Shutdown();
	ImGui::DestroyContext();

	window.Shutdown();
	ng::Shutdown();

	delete theGame;
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

	ImGui::End();
}
