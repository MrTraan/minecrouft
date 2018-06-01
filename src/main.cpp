#include <glad/glad.h>
#include <vector>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
#include <Texture.hpp>
#include <Window.hpp>
#include <cassert>

constexpr char windowName[] = "Minecrouft";

#include <unistd.h>
int main(void) {
	Window window;
	Camera camera(glm::vec3(0.0f, CHUNK_SIZE, 0.0f));
#ifdef WIN32
	Shader shader("C:\\Users\\nathan\\cpp\\minecrouft\\shaders\\vertex.glsl",
	              "C:\\Users\\nathan\\cpp\\minecrouft\\shaders\\fragment.glsl");
#else
	Shader shader("../shaders/vertex.glsl", "../shaders/fragment.glsl");
#endif



	// Setup imgui
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	ImGui_ImplGlfwGL3_Init(window.GetGlfwWindow(), false);

	Keyboard::Init(window);
	Mouse::Init(window);

	glm::mat4 proj = glm::perspective(
	    glm::radians(45.0f), (float)window.Width / window.Height, 0.1f, 400.0f);
	glm::mat4 model = glm::mat4();
	glm::mat4 view;

	Frustrum frustrum(proj);
	ChunkManager chunkManager(camera.Position, &frustrum);

	float dt = 0.0f;
	float lastFrame = 0.0f;

	char buffer[1000];
	getcwd(buffer, 1000);
	printf("cwd: %s\n", buffer);

	int major, minor, version;
	glfwGetVersion(&major, &minor, &version);
	printf("Glfw version: %d.%d.%d\n", major, minor, version);
    printf("OpenGL version: %s\n", glGetString(GL_VERSION));
	printf("GLM version: %d\n", GLM_VERSION);



	while (!window.ShouldClose()) {
		float currentFrame = glfwGetTime();
		dt = currentFrame - lastFrame;
		lastFrame = currentFrame;

		window.Clear();
		window.PollEvents();
		window.ProcessInput();
		ImGui_ImplGlfwGL3_NewFrame();

		Mouse::Update();

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
		            1000.0f / ImGui::GetIO().Framerate,
		            ImGui::GetIO().Framerate);

		camera.Update(dt);
		view = camera.GetViewMatrix();
		frustrum.Update(view);

		chunkManager.Update(camera.Position);

		shader.Use();

		int viewLoc = glGetUniformLocation(shader.ID, "view");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		int projLoc = glGetUniformLocation(shader.ID, "projection");
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

		int modelLoc = glGetUniformLocation(shader.ID, "model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

		chunkManager.Draw(shader);

		ImGui::Render();
		ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());

		window.SwapBuffers();
		auto err = glGetError();
		assert(err == GL_NO_ERROR);

	}

	ImGui_ImplGlfwGL3_Shutdown();
	ImGui::DestroyContext();

	return 0;
}
