#include "Hud.hpp"
#include <glad/glad.h>
#include "glm/gtc/type_ptr.hpp"

static constexpr float reticuleLines[] = {
    0.0f, 0.2f, 0.0f,     1.0f, 0.0f, 0.0f,
	0.0f, -0.2f, 0.0f,    1.0f, 0.0f, 0.0f,
    0.2f, 0.0f, 0.0f,     1.0f, 0.0f, 0.0f,
	-0.2f, 0.0f, 0.0f,    1.0f, 0.0f, 0.0f,
};

void Hud::Init() {
	shader.CompileFromPath( "resources/shaders/colored_vertex.glsl", "resources/shaders/colored_fragment.glsl" );
	glGenVertexArrays( 1, &VAO );
	glGenBuffers( 1, &VBO );
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray( VAO );

	glBindBuffer( GL_ARRAY_BUFFER, VBO );
	glBufferData( GL_ARRAY_BUFFER, sizeof( reticuleLines ), reticuleLines, GL_STATIC_DRAW );

	// position attribute
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof( float ), ( void * )0 );
	glEnableVertexAttribArray( 0 );
	// color attribute
	glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof( float ), ( void * )( 3 * sizeof( float ) ) );
	glEnableVertexAttribArray( 1 );
}

void Hud::Draw() {
	shader.Use();
	int viewLoc = glGetUniformLocation( shader.ID, "view" );
	glUniformMatrix4fv( viewLoc, 1, GL_FALSE, glm::value_ptr( glm::mat4(1.0f)) );
	int projLoc = glGetUniformLocation( shader.ID, "projection" );
	glUniformMatrix4fv( projLoc, 1, GL_FALSE, glm::value_ptr( glm::mat4(1.0f) ) );
	glBindVertexArray( VAO );
	glDrawArrays( GL_LINE_STRIP, 0, 2 );
	glDrawArrays( GL_LINE_STRIP, 2, 2 );
}

void Hud::Shutdown() {
	glDeleteBuffers( 1, &VBO );
	glDeleteVertexArrays( 1, &VAO );
}
