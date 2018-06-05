#include <glad/glad.h>

#include "Mesh.hpp"

void Mesh::InitMesh() {
	glGenVertexArrays(1, &(this->VAO));
	glGenBuffers(1, &(this->VBO));
	glGenBuffers(1, &(this->EBO));

	glBindVertexArray(this->VAO);
	glBindBuffer(GL_ARRAY_BUFFER, this->VBO);

	glBufferData(GL_ARRAY_BUFFER, VerticesCount * sizeof(Vertex),
	             &(this->Vertices[0]), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndicesCount * sizeof(unsigned int),
	             &(this->Indices[0]), GL_STATIC_DRAW);

	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

	// texture coords
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
	                      (void*)offsetof(Vertex, TexCoords));

	glBindVertexArray(0);
	this->isInit = true;
}

void Mesh::Draw(Shader shader) {
	if (!this->isInit)
		this->InitMesh();
	shader.Use();
	glBindVertexArray(this->VAO);
	glDrawElements(GL_TRIANGLES, IndicesCount, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
