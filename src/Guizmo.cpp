#include "Guizmo.hpp"
#include "Camera.hpp"
#include "Shader.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <vector>
#include "tracy/Tracy.hpp"

namespace Guizmo {

static Shader colorShader;
static u32    VAO, VBO;

struct LineData {
	glm::vec3 a;
	glm::vec3 colorA;
	glm::vec3 b;
	glm::vec3 colorB;
};

static std::vector< LineData > lineDrawList;

void Init() {
	colorShader.CompileFromPath( "resources/shaders/colored_vertex.glsl", "resources/shaders/colored_fragment.glsl" );

	glGenVertexArrays( 1, &VAO );
	glGenBuffers( 1, &VBO );
	glBindVertexArray( VAO );

	glBindBuffer( GL_ARRAY_BUFFER, VBO );
	glBufferData( GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW );

	// position attribute
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof( float ), ( void * )0 );
	glEnableVertexAttribArray( 0 );
	// color attribute
	glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof( float ), ( void * )( 3 * sizeof( float ) ) );
	glEnableVertexAttribArray( 1 );
}

void NewFrame() { lineDrawList.clear(); }

void Line( glm::vec3 a, glm::vec3 b, glm::vec3 color ) {
	LineData data = {a, color, b, color};
	lineDrawList.push_back( data );
}

void LinesAroundCube( glm::vec3 cubePosition ) {
	float       x = cubePosition.x;
	float       y = cubePosition.y;
	float       z = cubePosition.z;
	float     o = 0.001f;
	glm::vec3 ov( -o, -o, -o );

	Guizmo::Line( glm::vec3( x, y, z ) + ov, glm::vec3( x + 1 + 2 * o, y, z ) + ov, Guizmo::colWhite );
	Guizmo::Line( glm::vec3( x, y, z ) + ov, glm::vec3( x, y, z + 1 + 2 * o ) + ov, Guizmo::colWhite );
	Guizmo::Line( glm::vec3( x, y, z + 1 + 2 * o ) + ov, glm::vec3( x + 1 + 2 * o, y, z + 1 + 2 * o ) + ov,
	              Guizmo::colWhite );
	Guizmo::Line( glm::vec3( x + 1 + 2 * o, y, z ) + ov, glm::vec3( x + 1 + 2 * o, y, z + 1 + 2 * o ) + ov,
	              Guizmo::colWhite );

	Guizmo::Line( glm::vec3( x, y + 1 + 2 * o, z ) + ov, glm::vec3( x + 1 + 2 * o, y + 1 + 2 * o, z ) + ov,
	              Guizmo::colWhite );
	Guizmo::Line( glm::vec3( x, y + 1 + 2 * o, z ) + ov, glm::vec3( x, y + 1 + 2 * o, z + 1 + 2 * o ) + ov,
	              Guizmo::colWhite );
	Guizmo::Line( glm::vec3( x, y + 1 + 2 * o, z + 1 + 2 * o ) + ov,
	              glm::vec3( x + 1 + 2 * o, y + 1 + 2 * o, z + 1 + 2 * o ) + ov, Guizmo::colWhite );
	Guizmo::Line( glm::vec3( x + 1 + 2 * o, y + 1 + 2 * o, z ) + ov,
	              glm::vec3( x + 1 + 2 * o, y + 1 + 2 * o, z + 1 + 2 * o ) + ov, Guizmo::colWhite );

	Guizmo::Line( glm::vec3( x, y, z ) + ov, glm::vec3( x, y + 1 + 2 * o, z ) + ov, Guizmo::colWhite );
	Guizmo::Line( glm::vec3( x + 1 + 2 * o, y, z ) + ov, glm::vec3( x + 1 + 2 * o, y + 1 + 2 * o, z ) + ov,
	              Guizmo::colWhite );
	Guizmo::Line( glm::vec3( x, y, z + 1 + 2 * o ) + ov, glm::vec3( x, y + 1 + 2 * o, z + 1 + 2 * o ) + ov,
	              Guizmo::colWhite );
	Guizmo::Line( glm::vec3( x + 1 + 2 * o, y, z + 1 + 2 * o ) + ov,
	              glm::vec3( x + 1 + 2 * o, y + 1 + 2 * o, z + 1 + 2 * o ) + ov, Guizmo::colWhite );
}

void Draw( const Camera & camera ) {
	ZoneScoped;
	if ( lineDrawList.size() == 0 )
		return;

	colorShader.Use();
	int viewLoc = glGetUniformLocation( colorShader.ID, "view" );
	glUniformMatrix4fv( viewLoc, 1, GL_FALSE, glm::value_ptr( camera.viewMatrix ) );
	int projLoc = glGetUniformLocation( colorShader.ID, "projection" );
	glUniformMatrix4fv( projLoc, 1, GL_FALSE, glm::value_ptr( camera.projMatrix ) );

	glBindVertexArray( VAO );
	glBindBuffer( GL_ARRAY_BUFFER, VBO );
	glBufferData( GL_ARRAY_BUFFER, lineDrawList.size() * sizeof( LineData ), &( lineDrawList[ 0 ] ), GL_STATIC_DRAW );

	for ( int i = 0; i < lineDrawList.size(); i++ ) {
		glDrawArrays( GL_LINE_STRIP, i * 2, 2 );
	}
	glBindVertexArray( 0 );
}

} // namespace Guizmo