#include <glad/glad.h>

#include "Mesh.hpp"
#include "ngLib/nglib.h"
#include <tracy/Tracy.hpp>
#include <tracy/TracyOpenGL.hpp>

void meshCreateGLBuffers( Mesh * mesh ) {
	ZoneScoped;
	TracyGpuZone( "meshCreateGLBufferGPU" );
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
	TracyGpuZone( "meshDraw" );
	glBindVertexArray( mesh->VAO );
	glDrawElements( GL_TRIANGLES, mesh->IndicesCount, GL_UNSIGNED_INT, 0 );
	glBindVertexArray( 0 );
}

constexpr Vertex unitCubeVertices[ 12 ] = {
    {{0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, 0}, {{1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, 0}, {{1.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, 0},
    {{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, 0}, {{0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, 0}, {{1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, 0},
    {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}, 0}, {{0.0f, 1.0f, 1.0f}, {1.0f, 0.0f}, 0}, {{1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, 0},
    {{0.0f, 1.0f, 1.0f}, {0.0f, 1.0f}, 0}, {{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, 0}, {{1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, 0},
};

constexpr u32 unitCubeIndices[ 36 ] = {
    0, 2, 1, 0, 3, 2, 4, 5, 6, 4, 6, 7, 0, 4, 7, 0, 7, 3, 1, 6, 5, 1, 2, 6, 3, 8, 2, 3, 9, 8, 0, 1, 11, 0, 11, 10,
};

void PrepareTexturedUnitCube( Mesh * mesh, float textureIndex ) {
	mesh->Indices = ( u32 * )ng_alloc( sizeof( u32 ) * 36 );
	ng_assert( mesh->Indices != NULL );
	mesh->Vertices = ( Vertex * )ng_alloc( sizeof( Vertex ) * 12 );
	ng_assert( mesh->Vertices != NULL );
	memcpy( mesh->Vertices, unitCubeVertices, sizeof( unitCubeVertices ) );
	memcpy( mesh->Indices, unitCubeIndices, sizeof( unitCubeIndices ) );
	mesh->IndicesCount = 36;
	mesh->VerticesCount = 12;
	UpdateTextureIndexOnUnitCube( mesh, textureIndex );
}

void UpdateTextureIndexOnUnitCube( Mesh * mesh, float textureIndex ) {
	ng_assert( mesh->VerticesCount == 12 );
	for ( int i = 0; i < 12; i++ ) {
		mesh->Vertices[ i ].TexIndex = textureIndex;
	}
}
