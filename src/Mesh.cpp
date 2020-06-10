#include <glad/glad.h>

#include "Mesh.hpp"
#include "ngLib/nglib.h"
#include <tracy/Tracy.hpp>
#include <tracy/TracyOpenGL.hpp>

void meshCreateGLBuffers( VoxelMesh * mesh ) {
	ZoneScoped;
	TracyGpuZone( "meshCreateGLBufferGPU" );
	glGenVertexArrays( 1, &( mesh->VAO ) );
	glGenBuffers( 1, &( mesh->VBO ) );

	glBindVertexArray( mesh->VAO );
	glBindBuffer( GL_ARRAY_BUFFER, mesh->VBO );
	glBufferData( GL_ARRAY_BUFFER, mesh->verticesCount * sizeof( VoxelVertex ), &( mesh->Vertices[ 0 ] ),
	              GL_STATIC_DRAW );

	glEnableVertexAttribArray( 0 );
	glVertexAttribIPointer( 0, 1, GL_UNSIGNED_BYTE, sizeof( VoxelVertex ), ( void * )offsetof( VoxelVertex, x ) );

	glEnableVertexAttribArray( 1 );
	glVertexAttribIPointer( 1, 1, GL_UNSIGNED_BYTE, sizeof( VoxelVertex ), ( void * )offsetof( VoxelVertex, z ) );

	glEnableVertexAttribArray( 2 );
	glVertexAttribIPointer( 2, 1, GL_UNSIGNED_SHORT, sizeof( VoxelVertex ), ( void * )offsetof( VoxelVertex, y ) );

	// texture index
	glEnableVertexAttribArray( 3 );
	glVertexAttribIPointer( 3, 1, GL_UNSIGNED_BYTE, sizeof( VoxelVertex ),
	                        ( void * )offsetof( VoxelVertex, texIndex ) );

	// texture coords
	glEnableVertexAttribArray( 4 );
	glVertexAttribIPointer( 4, 1, GL_UNSIGNED_BYTE, sizeof( VoxelVertex ), ( void * )offsetof( VoxelVertex, texX ) );

	glEnableVertexAttribArray( 5 );
	glVertexAttribIPointer( 5, 1, GL_UNSIGNED_SHORT, sizeof( VoxelVertex ), ( void * )offsetof( VoxelVertex, texY ) );

	glEnableVertexAttribArray( 6 );
	glVertexAttribIPointer( 6, 1, GL_UNSIGNED_BYTE, sizeof( VoxelVertex ),
	                        ( void * )offsetof( VoxelVertex, direction ) );

	glBindVertexArray( 0 );
}

void meshUpdateBuffer( VoxelMesh * mesh ) {
	glBindVertexArray( mesh->VAO );
	glBindBuffer( GL_ARRAY_BUFFER, mesh->VBO );
	glBufferData( GL_ARRAY_BUFFER, mesh->verticesCount * sizeof( VoxelVertex ), &( mesh->Vertices[ 0 ] ),
	              GL_STATIC_DRAW );
	glBindVertexArray( 0 );
}

void meshDeleteBuffers( VoxelMesh * mesh ) {
	glDeleteBuffers( 1, &( mesh->VBO ) );
	glDeleteVertexArrays( 1, &( mesh->VAO ) );
}

void meshDraw( VoxelMesh * mesh ) {
	TracyGpuZone( "meshDraw" );
	glBindVertexArray( mesh->VAO );
	glDrawArrays( GL_TRIANGLES, 0, mesh->verticesCount );
	glBindVertexArray( 0 );
}

void meshDrawWireframe( VoxelMesh * mesh ) {
	TracyGpuZone( "meshDrawWireframe" );
	glBindVertexArray( mesh->VAO );
	glDrawArrays( GL_LINES, 0, mesh->verticesCount );
	glBindVertexArray( 0 );
}

// constexpr Vertex unitCubeVertices[ 12 ] = {
//    {{0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, 0}, {{1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, 0}, {{1.0f, 1.0f, 0.0f}, {1.0f, 0.0f},
//    0},
//    {{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, 0}, {{0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, 0}, {{1.0f, 0.0f, 1.0f}, {0.0f, 1.0f},
//    0},
//    {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}, 0}, {{0.0f, 1.0f, 1.0f}, {1.0f, 0.0f}, 0}, {{1.0f, 1.0f, 1.0f}, {1.0f, 1.0f},
//    0},
//    {{0.0f, 1.0f, 1.0f}, {0.0f, 1.0f}, 0}, {{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, 0}, {{1.0f, 0.0f, 1.0f}, {1.0f, 0.0f},
//    0},
//};
//
// constexpr u8 unitCubeIndices[ 36 ] = {
//    0, 2, 1, 0, 3, 2, 4, 5, 6, 4, 6, 7, 0, 4, 7, 0, 7, 3, 1, 6, 5, 1, 2, 6, 3, 8, 2, 3, 9, 8, 0, 1, 11, 0, 11, 10,
//};

void PrepareTexturedUnitCube( VoxelMesh * mesh, float textureIndex ) {
	// mesh->Indices = ( u8 * )ng_alloc( sizeof( u8 ) * 36 );
	// ng_assert( mesh->Indices != NULL );
	// mesh->Vertices = ( Vertex * )ng_alloc( sizeof( Vertex ) * 12 );
	// ng_assert( mesh->Vertices != NULL );
	// memcpy( mesh->Vertices, unitCubeVertices, sizeof( unitCubeVertices ) );
	// memcpy( mesh->Indices, unitCubeIndices, sizeof( unitCubeIndices ) );
	// mesh->IndicesCount = 36;
	// mesh->VerticesCount = 12;
	// UpdateTextureIndexOnUnitCube( mesh, textureIndex );
}

void UpdateTextureIndexOnUnitCube( VoxelMesh * mesh, float textureIndex ) {
	// ng_assert( mesh->VerticesCount == 12 );
	// for ( int i = 0; i < 12; i++ ) {
	//	mesh->Vertices[ i ].TexIndex = textureIndex;
	//}
}
