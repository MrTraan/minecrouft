#include <glad/glad.h>
#include <vector>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <tracy/Tracy.hpp>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw_gl3.h>

#include <Camera.hpp>
#include <Chunk.hpp>
#include <ChunkManager.hpp>
#include <Frustrum.hpp>
#include <Keyboard.hpp>
#include <Mesh.hpp>
#include <Mouse.hpp>
#include <Shader.hpp>
#include <Window.hpp>
#include <cassert>
#include <Skybox.hpp>
#include <LuaConsole.hpp>

#define GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX 0x9048
#define GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX 0x9049

void GLAPIENTRY
MessageCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
		return;
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
		type, severity, message);
}

constexpr char windowName[] = "Minecrouft";

int main(void) {
	Window window;
	Camera camera( (float)window.Width / window.Height, glm::vec3(SHRT_MAX / 2, 180, SHRT_MAX / 2));

	// Setup imgui
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	ImGui_ImplGlfwGL3_Init(window.GetGlfwWindow(), false);

	Keyboard::Init(window);
	Mouse::Init(window);

	glm::mat4 model = glm::mat4(1.0);
	glm::mat4 view = glm::mat4(1.0);

	Frustrum frustrum(camera.projMatrix	);
	ChunkManager chunkManager(camera.Position, &frustrum);
	Skybox skybox;

	float dt = 0.0f;
	float lastFrame = 0.0f;

	int major, minor, version;
	glfwGetVersion(&major, &minor, &version);
	const GLubyte* vendor = glGetString(GL_VENDOR);
	printf("Vendor name: %s\n", vendor);
	std::string sVendor = (char*)vendor;
	std::string sNvidia = "NVIDIA";
	bool isGraphicCardNvidia = sVendor.find(sNvidia) != std::string::npos;
	printf("Glfw version: %d.%d.%d\n", major, minor, version);
	printf("OpenGL version: %s\n", glGetString(GL_VERSION));
	printf("GLM version: %d\n", GLM_VERSION);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	bool isConsoleOpen = true;
	while (!window.ShouldClose()) {
		ZoneScopedN( "MainLoop" );
		float currentFrame = (float)glfwGetTime();
		dt = currentFrame - lastFrame;
		lastFrame = currentFrame;

		window.Clear();
		window.PollEvents();
		window.ProcessInput();
		ImGui_ImplGlfwGL3_NewFrame();

		if (isConsoleOpen)
			ShowLuaConsole(&isConsoleOpen);

		Mouse::Update();

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
		            1000.0f / ImGui::GetIO().Framerate,
		            ImGui::GetIO().Framerate);

		camera.Update(dt);
		view = camera.GetViewMatrix();
		skybox.Draw(view, camera.projMatrix	);
		frustrum.Update(view);

		chunkManager.Update(camera.Position);

		chunkManager.Draw( camera );

		if (isGraphicCardNvidia) {
			GLint total_mem_kb = 0;
			glGetIntegerv(GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX,
			              &total_mem_kb);

			GLint cur_avail_mem_kb = 0;
			glGetIntegerv(GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX,
			              &cur_avail_mem_kb);
			ImGui::Text("Vram: %dmb / %dmb",
			            (total_mem_kb - cur_avail_mem_kb) / 1000,
			            total_mem_kb / 1000);
		}


		ImGui::Render();
		ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());

		window.SwapBuffers();

		FrameMark;
	}

	ImGui_ImplGlfwGL3_Shutdown();
	ImGui::DestroyContext();

	return 0;
}
