#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw_gl3.h>

#include <Keyboard.hpp>
#include <Mouse.hpp>
#include <vector>
#include "Camera.hpp"
#include "Mesh.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "Window.hpp"

#include <Chunk.hpp>
#include <Cube.hpp>

constexpr char windowName[] = "Minecrouft";

int main(void) {
	Window window;
	Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
	Shader shader("../shaders/vertex.glsl", "../shaders/fragment.glsl");

	// Setup imgui
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	ImGui_ImplGlfwGL3_Init(window.GetGlfwWindow(), false);

	Keyboard::Init(window);
	Mouse::Init(window);

	Chunk cube(glm::vec3(0.0f, 0.0f, 0.0f));

	glm::mat4 proj = glm::perspective(
	    glm::radians(45.0f), (float)window.Width / window.Height, 0.1f, 100.0f);
	glm::mat4 view;
	glm::mat4 model;

	float dt = 0.0f;
	float lastFrame = 0.0f;

	Texture texture("../resources/dirt.png", eImageFormat::RGB);

	int maxY = 10;
	int maxX = 10;

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
		ImGui::SliderInt("Max X", &maxX, 0, 100);
		ImGui::SliderInt("Max Y", &maxY, 0, 100);

		camera.Update(dt);
		view = camera.GetViewMatrix();

		shader.Use();
		texture.Bind();


		model = glm::mat4();
		model = glm::translate(model, glm::vec3(0.f, -3.0f, -3.0f));

		int viewLoc = glGetUniformLocation(shader.ID, "view");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		int projLoc = glGetUniformLocation(shader.ID, "projection");
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

		int modelLoc = glGetUniformLocation(shader.ID, "model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		cube.Draw(shader);

		ImGui::Render();
		ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());

		window.SwapBuffers();
	}

	ImGui_ImplGlfwGL3_Shutdown();
	ImGui::DestroyContext();

	return 0;
}
