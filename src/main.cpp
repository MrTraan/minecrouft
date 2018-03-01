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
		{ glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.25f, 0.333334f) },
		{ glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.5f, 0.333334f) },
		{ glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.5f, 0.666667f) },
		{ glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.25f, 0.666667f) },

		{ glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.5f, 0.333334f) },
		{ glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.25f, 0.333334f) },
		{ glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.25f, 0.666667f) },
		{ glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.5f, 0.666667f) },
	};

	std::vector<unsigned int> indices;
	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(0);
	indices.push_back(2);
	indices.push_back(3);
	indices.push_back(1);
	indices.push_back(5);
	indices.push_back(2);
	indices.push_back(2);
	indices.push_back(6);
	indices.push_back(5);
	indices.push_back(5);
	indices.push_back(7);
	indices.push_back(6);
	indices.push_back(4);
	indices.push_back(7);
	indices.push_back(5);
	indices.push_back(3);
	indices.push_back(4);
	indices.push_back(7);
	indices.push_back(0);
	indices.push_back(4);
	indices.push_back(3);
	indices.push_back(7);

	Mesh mesh(vertices, indices);

	glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)window.Width / window.Height, 0.1f, 100.0f);
	glm::mat4 view;
	glm::mat4 model;

	float dt = 0.0f;
	float lastFrame = 0.0f;

	Texture texture("C:\\Users\\natha\\Code\\minecraft\\resources\\dirt.png", eImageFormat::RGB);

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


		camera.Update(dt);
		view = camera.GetViewMatrix();
		//model = glm::rotate(model, glm::radians(1.0f), glm::vec3(0.5f, 1.0f, 0.0f));

		shader.Use();
		texture.Bind();

		int modelLoc = glGetUniformLocation(shader.ID, "model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		
		int viewLoc = glGetUniformLocation(shader.ID, "view");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		int projLoc = glGetUniformLocation(shader.ID, "projection");
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

		mesh.Draw(shader);

		ImGui::Render();
		ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());

		window.SwapBuffers();
    }

    return 0;
}
