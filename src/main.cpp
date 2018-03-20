#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw_gl3.h>

#include "Camera.hpp"
#include "Window.hpp"
#include "Shader.hpp"
#include "Mesh.hpp"
#include "Texture.hpp"
#include <Keyboard.hpp>
#include <Mouse.hpp>
#include <vector>

constexpr char windowName[] = "Minecrouft";

int main(void) {
	Window window;
	Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
	Shader shader(
		"C:\\Users\\natha\\Code\\minecraft\\shaders\\vertex.glsl",
		"C:\\Users\\natha\\Code\\minecraft\\shaders\\fragment.glsl"
	);

	// Setup imgui
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui_ImplGlfwGL3_Init(window.GetGlfwWindow(), false);

	Keyboard::Init(window);
	Mouse::Init(window);

	std::vector<Vertex> vertices = {
		{ glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.25f, 0.333334f) }, // 0
		{ glm::vec3(1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.5f, 0.333334f) }, // 1
		{ glm::vec3(1.0f, 1.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.5f, 0.666667f) }, // 2
		{ glm::vec3(-1.0f, 1.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.25f, 0.666667f) }, // 3

		{ glm::vec3(-1.0f, -1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.5f, 0.333334f) }, // 4
		{ glm::vec3(1.0f, -1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.25f, 0.333334f) }, // 5
		{ glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.25f, 0.666667f) }, // 6
		{ glm::vec3(-1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.5f, 0.666667f) }, // 7

		{ glm::vec3(1.0f, 1.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.5f, 1.0f) }, // 8
		{ glm::vec3(-1.0f, 1.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.25f, 1.0f) }, // 9
		{ glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.5f, 0.6666667f) }, // 10
		{ glm::vec3(-1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.25f, 0.6666667f) }, // 11

		{ glm::vec3(1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.5f, 0.333334f) }, // 12
		{ glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.25f, 0.333334f) }, // 13
		{ glm::vec3(1.0f, -1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.5f, 0.0f) }, // 14
		{ glm::vec3(-1.0f, -1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.25f, 0.0f) }, // 15
	};

	std::vector<unsigned int> indices = {
		0, 1, 2,
		0, 2, 3,
		1, 5, 2,
		2, 6, 5,
		5, 7, 6,
		4, 7, 5,
		3, 4, 7,
		0, 4, 3,
		9, 11, 8,
		8, 10, 11,
		12, 13, 14,
		13, 14 , 15,
	};

	Mesh mesh(vertices, indices);

	glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)window.Width / window.Height, 0.1f, 100.0f);
	glm::mat4 view;
	glm::mat4 model;

	float dt = 0.0f;
	float lastFrame = 0.0f;

	Texture texture("C:\\Users\\natha\\Code\\minecraft\\resources\\dirt.png", eImageFormat::RGB);
		
	int maxY = 10;
	int maxX = 10;

    while (!window.ShouldClose())
    {
		float currentFrame = glfwGetTime();
		dt = currentFrame - lastFrame;
		lastFrame = currentFrame;

		window.Clear();
		window.PollEvents();
		window.ProcessInput();
		ImGui_ImplGlfwGL3_NewFrame();

		Mouse::Update();

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::SliderInt("Max X", &maxX, 0, 100);
		ImGui::SliderInt("Max Y", &maxY, 0, 100);

		camera.Update(dt);
		view = camera.GetViewMatrix();
		//model = glm::rotate(model, glm::radians(1.0f), glm::vec3(0.5f, 1.0f, 0.0f));

		shader.Use();
		texture.Bind();


		model = glm::mat4();
		model = glm::translate(model, glm::vec3(0.f, -3.0f, -3.0f));
		
		int viewLoc = glGetUniformLocation(shader.ID, "view");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		int projLoc = glGetUniformLocation(shader.ID, "projection");
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

		for (int y = 0; y < maxY; y++) {
			for (int x = 0; x < maxX; x++) {
				model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));
				int modelLoc = glGetUniformLocation(shader.ID, "model");
				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
				mesh.Draw(shader);
			}
			model = glm::translate(model, glm::vec3(-2.0f * maxX, 0.0f, 2.0f));
		}

		ImGui::Render();
		ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());

		window.SwapBuffers();
    }

	ImGui_ImplGlfwGL3_Shutdown();
	ImGui::DestroyContext();

    return 0;
}
