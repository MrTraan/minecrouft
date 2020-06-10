#include "Hud.hpp"
#include "Game.h"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "packer_resource_list.h"
#include <GL/gl3w.h>

constexpr glm::vec3 red = {1.0f, 0.0f,0.0f};
constexpr glm::vec3 green = {0.0f, 1.0f,0.0f};
constexpr glm::vec3 blue = {0.0f, 0.0f,1.0f};

struct ReticuleLine {
	glm::vec3 positions;
	glm::vec3 color;
};

static constexpr ReticuleLine reticuleLines[] = {
	{ { 0.0f, 0.0f, 0.0f }, red },
	{ { 1.0f, 0.0f, 0.0f }, red },
	{ { 0.0f, 0.0f, 0.0f }, green },
	{ { 0.0f, 1.0f, 0.0f }, green },
	{ { 0.0f, 0.0f, 0.0f }, blue },
	{ { 0.0f, 0.0f, 1.0f }, blue },
};

void Hud::Init( int screenWidth, int screenHeight ) {
	orthoProjection = glm::ortho( 0.0f, ( float )screenWidth, ( float )screenHeight, 0.0f, -50.0f, 50.0f );

	shader.CompileFromResource( SHADERS_UI_VERT, SHADERS_UI_FRAG );
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
	shader.SetMatrix( "projection", orthoProjection );

	const Camera & cam = theGame->camera;
	glm::mat4 model(1.0f);
	model = glm::translate( model, glm::vec3( 50, 750, 0));
	glm::mat4 rotation = glm::yawPitchRoll( glm::radians(cam.yaw), glm::radians(cam.pitch), 0.0f );
	model = model * rotation;
	model = glm::scale( model, glm::vec3( 30, 30, 30));
	shader.SetMatrix( "model", model );

	glBindVertexArray( VAO );

	glLineWidth( 5.0f );
	glDrawArrays( GL_LINE_STRIP, 0, 2 );
	glDrawArrays( GL_LINE_STRIP, 2, 2 );
	glDrawArrays( GL_LINE_STRIP, 4, 2 );
	glLineWidth( 0.0f );
}

void Hud::Shutdown() {
	glDeleteBuffers( 1, &VBO );
	glDeleteVertexArrays( 1, &VAO );
}
