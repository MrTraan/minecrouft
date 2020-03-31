#include <glad/glad.h>
#include "Hud.hpp"

void Hud::Draw() {
	glEnableClientState( GL_VERTEX_ARRAY );
	glVertexPointer( 3, GL_FLOAT, 0, reticuleLines );
	glDrawArrays( GL_LINE_LOOP, 0, 2 );
	glDisableClientState( GL_VERTEX_ARRAY );
}
