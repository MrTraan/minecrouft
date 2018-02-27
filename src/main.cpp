#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Camera.hpp"
#include "Window.hpp"
#include "Shader.hpp"
#include "Mesh.hpp"
#include <vector>

constexpr char windowName[] = "Minecrouft";

int main(void) {
	Window window;
	Camera camera;
	Shader shader(
		"C:\\Users\\natha\\Code\\minecraft\\shaders\\vertex.glsl",
		"C:\\Users\\natha\\Code\\minecraft\\shaders\\fragment.glsl"
	);



		//0.5f, 0.5f, 0.0f,  // top right
		//0.5f, -0.5f, 0.0f,  // bottom right
		//-0.5f, -0.5f, 0.0f,  // bottom left
		//-0.5f, 0.5f, 0.0f   // top left 

	std::vector<Vertex> vertices = {
		{ glm::vec3(0.5f, 0.5f, 0.0f) },
		{ glm::vec3(0.5f, -0.5f, 0.0f) },
		{ glm::vec3(-0.5f, -0.5f, 0.0f) },
		{ glm::vec3(-0.5f, 0.5f, 0.0f) },
	};

	std::vector<unsigned int> indices;
	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(0);
	indices.push_back(2);
	indices.push_back(3);

	
	Mesh mesh(vertices, indices);

	glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)window.Width / window.Height, 0.1f, 100.0f);
	glm::mat4 model;
	glm::mat4 view;
	view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));

    while (!window.ShouldClose())
    {
		window.Clear();
		window.PollEvents();
		window.ProcessInput();

		shader.Use();
		int modelLoc = glGetUniformLocation(shader.ID, "model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		
		int viewLoc = glGetUniformLocation(shader.ID, "view");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		int projLoc = glGetUniformLocation(shader.ID, "projection");
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

		mesh.Draw(shader);

		window.SwapBuffers();
    }

    return 0;
}
