#include <glad/glad.h>

#include "Mesh.hpp"
#include <tracy/Tracy.hpp>
#include <tracy/TracyOpenGL.hpp>

void meshCreateGLBuffers( Mesh * mesh ) {
	ZoneScoped;
	TracyGpuZone("meshCreateGLBufferGPU");
	glGenVertexArrays( 1, &( mesh->VAO ) );
	glGenBuffers( 1, &( mesh->VBO ) );
	glGenBuffers( 1, &( mesh->EBO ) );

	glBindVertexArray( mesh->VAO );
	glBindBuffer( GL_ARRAY_BUFFER, mesh->VBO );
	glBufferData( GL_ARRAY_BUFFER, mesh->VerticesCount * sizeof( Vertex ), &( mesh->Vertices[ 0 ] ), GL_STATIC_DRAW );

	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mesh->EBO );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, mesh->IndicesCount * sizeof( u32 ), &( mesh->Indices[ 0 ] ),
	              GL_STATIC_DRAW );

	// vertex positions
	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( Vertex ), ( void * )0 );

	// texture coords
	glEnableVertexAttribArray( 1 );
	glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, sizeof( Vertex ), ( void * )offsetof( Vertex, TexCoords ) );

	// texture index
	glEnableVertexAttribArray( 2 );
	glVertexAttribPointer( 2, 1, GL_FLOAT, GL_FALSE, sizeof( Vertex ), ( void * )offsetof( Vertex, TexIndex ) );
	glBindVertexArray( 0 );
}

void meshUpdateBuffer( Mesh * mesh ) {
	glBindVertexArray( mesh->VAO );
	glBindBuffer( GL_ARRAY_BUFFER, mesh->VBO );
	glBufferData( GL_ARRAY_BUFFER, mesh->VerticesCount * sizeof( Vertex ), &( mesh->Vertices[ 0 ] ), GL_STATIC_DRAW );

	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mesh->EBO );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, mesh->IndicesCount * sizeof( u32 ), &( mesh->Indices[ 0 ] ),
	              GL_STATIC_DRAW );
	glBindVertexArray( 0 );
}

void meshDeleteBuffers( Mesh * mesh ) {
	glDeleteBuffers( 1, &( mesh->EBO ) );
	glDeleteBuffers( 1, &( mesh->VBO ) );
	glDeleteVertexArrays( 1, &( mesh->VAO ) );
}

void meshDraw( Mesh * mesh ) {
	TracyGpuZone("meshDraw");
	glBindVertexArray( mesh->VAO );
	glDrawElements( GL_TRIANGLES, mesh->IndicesCount, GL_UNSIGNED_INT, 0 );
	glBindVertexArray( 0 );
}
