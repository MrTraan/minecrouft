#include <Chunk.hpp>
#include <ChunkManager.hpp>
#include <Debug.hpp>
#include <HeightMap.hpp>
#include <TextureAtlas.hpp>

#include "ngLib/nglib.h"
#include <constants.hpp>
#include <imgui/imgui.h>
#include <stdlib.h>
#include <tracy/Tracy.hpp>

static void pushFace( Mesh *     mesh,
                      glm::vec3  a,
                      glm::vec3  b,
                      glm::vec3  c,
                      glm::vec3  d,
                      float      width,
                      float      height,
                      eDirection direction,
                      int        reverse,
                      eBlockType type );

void chunkCreateGeometry( Chunk * chunk ) {
	ZoneScoped;
	static thread_local eBlockType mask[ CHUNK_SIZE * CHUNK_HEIGHT ];
	chunk->mesh->facesBuilt = 0;
	chunk->mesh->IndicesCount = 0;
	chunk->mesh->VerticesCount = 0;
	
	chunk->transparentMesh->facesBuilt = 0;
	chunk->transparentMesh->IndicesCount = 0;
	chunk->transparentMesh->VerticesCount = 0;

	int        dims[ 3 ] = {CHUNK_SIZE, CHUNK_HEIGHT, CHUNK_SIZE};
	glm::ivec3 p1, p2, p3, p4;

	for ( int reverse = 0; reverse < 2; reverse++ ) {
		for ( int d = 0; d < 3; d++ ) {
			int        u = ( d + 1 ) % 3;
			int        v = ( d + 2 ) % 3;
			eDirection dir;
			if ( d == 0 )
				dir = ( reverse ? eDirection::WEST : eDirection::EAST );
			else if ( d == 1 )
				dir = ( reverse ? eDirection::BOTTOM : eDirection::TOP );
			else if ( d == 2 )
				dir = ( reverse ? eDirection::SOUTH : eDirection::NORTH );

			int indices[ 3 ] = {0, 0, 0};
			int offset[ 3 ] = {0, 0, 0};
			offset[ d ] = 1;

			for ( indices[ d ] = -1; indices[ d ] < dims[ d ]; ) {
				// compute mask
				int n = 0;
				for ( indices[ v ] = 0; indices[ v ] < dims[ v ]; indices[ v ]++ ) {
					for ( indices[ u ] = 0; indices[ u ] < dims[ u ]; indices[ u ]++ ) {
						eBlockType face1 =
						    ( indices[ d ] >= 0 ? chunk->cubes[ indices[ 0 ] ][ indices[ 1 ] ][ indices[ 2 ] ]
						                        : eBlockType::INACTIVE );
						eBlockType face2 =
						    ( indices[ d ] < dims[ d ] - 1
						          ? chunk->cubes[ indices[ 0 ] + offset[ 0 ] ][ indices[ 1 ] + offset[ 1 ] ]
						                        [ indices[ 2 ] + offset[ 2 ] ]
						          : eBlockType::INACTIVE );

						mask[ n++ ] =
						    ( face1 != eBlockType::INACTIVE && face2 != eBlockType::INACTIVE && face1 == face2 )
						        ? eBlockType::INACTIVE
						        : ( reverse ? face2 : face1 );
					}
				}

				indices[ d ]++;
				n = 0;
				for ( int j = 0; j < dims[ v ]; j++ ) {
					for ( int i = 0; i < dims[ u ]; ) {
						if ( mask[ n ] != eBlockType::INACTIVE ) {
							int w, h;
							for ( w = 1; i + w < dims[ u ] && mask[ n + w ] != eBlockType::INACTIVE &&
							             mask[ n + w ] == mask[ n ];
							      ++w )
								;

							for ( h = 1; j + h < dims[ v ]; ++h ) {
								for ( int k = 0; k < w; k++ ) {
									if ( mask[ n + k + h * dims[ u ] ] == eBlockType::INACTIVE ||
									     mask[ n + k + h * dims[ u ] ] != mask[ n ] )
										goto BREAK_LOOP;
								}
							}
						BREAK_LOOP:

							indices[ u ] = i;
							indices[ v ] = j;
							int du[ 3 ] = {0, 0, 0};
							int dv[ 3 ] = {0, 0, 0};
							du[ u ] = w;
							dv[ v ] = h;
							p1.x = indices[ 0 ];
							p1.y = indices[ 1 ];
							p1.z = indices[ 2 ];
							p2.x = indices[ 0 ] + du[ 0 ];
							p2.y = indices[ 1 ] + du[ 1 ];
							p2.z = indices[ 2 ] + du[ 2 ];
							p3.x = indices[ 0 ] + du[ 0 ] + dv[ 0 ];
							p3.y = indices[ 1 ] + du[ 1 ] + dv[ 1 ];
							p3.z = indices[ 2 ] + du[ 2 ] + dv[ 2 ];
							p4.x = indices[ 0 ] + dv[ 0 ];
							p4.y = indices[ 1 ] + dv[ 1 ];
							p4.z = indices[ 2 ] + dv[ 2 ];

							if ( mask[ n ] == eBlockType::WATER ) {
								// ONLY DRAW TOP SURFACE
								 if ( dir == eDirection::TOP ) {
									if ( d == 0 )
										pushFace( chunk->transparentMesh, p4, p1, p2, p3, ( float )h, ( float )w, dir,
										          reverse, mask[ n ] );
									else
										pushFace( chunk->transparentMesh, p1, p2, p3, p4, ( float )w, ( float )h, dir,
										          reverse, mask[ n ] );
								}
							} else {
								if ( d == 0 )
									pushFace( chunk->mesh, p4, p1, p2, p3, ( float )h, ( float )w, dir, reverse,
									          mask[ n ] );
								else
									pushFace( chunk->mesh, p1, p2, p3, p4, ( float )w, ( float )h, dir, reverse,
									          mask[ n ] );
							}

							for ( int l = 0; l < h; l++ )
								for ( int k = 0; k < w; k++ )
									mask[ n + k + l * dims[ u ] ] = eBlockType::INACTIVE;

							i += w;
							n += w;
						} else {
							i++;
							n++;
						}
					}
				}
			}
		}
	}
}

// if ( chunk->facesAllocated > chunk->facesBuilt * 2 ) j
//	ZoneScopedN( "ShrunkingFacesAllocated" );
//	chunk->mesh->Indices = ( u32 * )realloc( chunk->mesh->Indices, sizeof( u32 ) * 6 * chunk->facesBuilt );
//	ng_assert( chunk->mesh->Indices != NULL );
//	chunk->mesh->Vertices = ( Vertex * )realloc( chunk->mesh->Vertices, sizeof( Vertex ) * 4 * chunk->facesBuilt );
//	ng_assert( chunk->mesh->Vertices != NULL );
//	chunk->facesAllocated = chunk->facesBuilt;
//}

void pushFace( Mesh *     mesh,
               glm::vec3  a,
               glm::vec3  b,
               glm::vec3  c,
               glm::vec3  d,
               float      width,
               float      height,
               eDirection direction,
               int        reverse,
               eBlockType type ) {
	int uvModifier;

	switch ( type ) {
	case eBlockType::GRASS:
		uvModifier = 0;
		break;
	case eBlockType::SNOW:
		uvModifier = 1;
		break;
	case eBlockType::ROCK:
		uvModifier = 2;
		break;
	case eBlockType::SAND:
		uvModifier = 3;
		break;
	case eBlockType::WATER:
		uvModifier = 4;
		break;
	default:
		uvModifier = 0;
		break;
	}

	while ( mesh->facesAllocated <= mesh->facesBuilt ) {
		ZoneScopedN( "GrowingFacesAllocated" );
		auto grownIndices = ( u32 * )ng_alloc( sizeof( u32 ) * 6 * ( FACES_BATCH_ALLOC + mesh->facesAllocated ) );
		ng_assert( grownIndices != nullptr );
		auto grownVertices =
		    ( Vertex * )ng_alloc( sizeof( Vertex ) * 4 * ( FACES_BATCH_ALLOC + mesh->facesAllocated ) );
		ng_assert( grownVertices != nullptr );
		memcpy( grownIndices, mesh->Indices, sizeof( u32 ) * 6 * mesh->facesAllocated );
		memcpy( grownVertices, mesh->Vertices, sizeof( Vertex ) * 4 * mesh->facesAllocated );
		ng_free( mesh->Indices );
		ng_free( mesh->Vertices );
		mesh->Indices = grownIndices;
		mesh->Vertices = grownVertices;
		mesh->facesAllocated += FACES_BATCH_ALLOC;
	}

	// Vertex index
	u32 vi = 4 * mesh->facesBuilt;
	// Indices index
	u32 ii = 6 * mesh->facesBuilt;

	mesh->facesBuilt++;

	mesh->IndicesCount += 6;
	if ( reverse ) {
		mesh->Indices[ ii + 0 ] = vi + 1;
		mesh->Indices[ ii + 1 ] = vi + 0;
		mesh->Indices[ ii + 2 ] = vi + 3;
		mesh->Indices[ ii + 3 ] = vi + 3;
		mesh->Indices[ ii + 4 ] = vi + 2;
		mesh->Indices[ ii + 5 ] = vi + 1;
	} else {
		mesh->Indices[ ii + 0 ] = vi + 1;
		mesh->Indices[ ii + 1 ] = vi + 2;
		mesh->Indices[ ii + 2 ] = vi + 3;
		mesh->Indices[ ii + 3 ] = vi + 3;
		mesh->Indices[ ii + 4 ] = vi + 0;
		mesh->Indices[ ii + 5 ] = vi + 1;
	}

	mesh->VerticesCount += 4;
	Vertex * v = mesh->Vertices;

	v[ vi + 0 ].TexCoords[ 0 ] = 0.0f;
	v[ vi + 0 ].TexCoords[ 1 ] = height;
	v[ vi + 1 ].TexCoords[ 0 ] = width;
	v[ vi + 1 ].TexCoords[ 1 ] = height;
	v[ vi + 2 ].TexCoords[ 0 ] = width;
	v[ vi + 2 ].TexCoords[ 1 ] = 0.0f;
	v[ vi + 3 ].TexCoords[ 0 ] = 0.0f;
	v[ vi + 3 ].TexCoords[ 1 ] = 0.0f;

	v[ vi + 0 ].Position[ 0 ] = a.x;
	v[ vi + 0 ].Position[ 1 ] = a.y;
	v[ vi + 0 ].Position[ 2 ] = a.z;

	v[ vi + 1 ].Position[ 0 ] = b.x;
	v[ vi + 1 ].Position[ 1 ] = b.y;
	v[ vi + 1 ].Position[ 2 ] = b.z;

	v[ vi + 2 ].Position[ 0 ] = c.x;
	v[ vi + 2 ].Position[ 1 ] = c.y;
	v[ vi + 2 ].Position[ 2 ] = c.z;

	v[ vi + 3 ].Position[ 0 ] = d.x;
	v[ vi + 3 ].Position[ 1 ] = d.y;
	v[ vi + 3 ].Position[ 2 ] = d.z;

	switch ( direction ) {
	case eDirection::TOP:
		for ( int i = 0; i < 4; i++ ) {
			v[ vi + i ].TexIndex = float( uvModifier * 3 );
		}
		break;
	case eDirection::BOTTOM:
		for ( int i = 0; i < 4; i++ ) {
			v[ vi + i ].TexIndex = float( uvModifier * 3 + 2 );
		}
		break;
	case eDirection::SOUTH:
	case eDirection::EAST:
	case eDirection::WEST:
	case eDirection::NORTH:
		for ( int i = 0; i < 4; i++ ) {
			v[ vi + i ].TexIndex = float( uvModifier * 3 + 1 );
		}
		break;
	}
}

Chunk * preallocateChunk() {
	ZoneScoped;
	Chunk * chunk = new Chunk();
	chunk->mesh = new Mesh();
	chunk->mesh->Indices = ( u32 * )ng_alloc( sizeof( u32 ) * 6 * FACES_INITIAL_ALLOC );
	ng_assert( chunk->mesh->Indices != NULL );
	chunk->mesh->Vertices = ( Vertex * )ng_alloc( sizeof( Vertex ) * 4 * FACES_INITIAL_ALLOC );
	ng_assert( chunk->mesh->Vertices != NULL );
	chunk->mesh->IndicesCount = 0;
	chunk->mesh->VerticesCount = 0;
	chunk->mesh->facesAllocated = FACES_INITIAL_ALLOC;
	chunk->mesh->facesBuilt = 0;

	chunk->transparentMesh = new Mesh();
	chunk->transparentMesh->Indices = ( u32 * )ng_alloc( sizeof( u32 ) * 6 * 8 );
	ng_assert( chunk->transparentMesh->Indices != NULL );
	chunk->transparentMesh->Vertices = ( Vertex * )ng_alloc( sizeof( Vertex ) * 4 * 8 );
	ng_assert( chunk->transparentMesh->Vertices != NULL );
	chunk->transparentMesh->IndicesCount = 0;
	chunk->transparentMesh->VerticesCount = 0;
	chunk->transparentMesh->facesAllocated = 8;
	chunk->transparentMesh->facesBuilt = 0;

	return chunk;
}

void chunkDestroy( Chunk * chunk ) {
	if ( chunk->mesh != nullptr ) {
		if ( chunk->mesh->Vertices )
			ng_free( chunk->mesh->Vertices );
		if ( chunk->mesh->Indices )
			ng_free( chunk->mesh->Indices );
	}
	delete chunk->mesh;
	if ( chunk->transparentMesh != nullptr ) {
		if ( chunk->transparentMesh->Vertices )
			ng_free( chunk->transparentMesh->Vertices );
		if ( chunk->transparentMesh->Indices )
			ng_free( chunk->transparentMesh->Indices );
	}
	delete chunk->transparentMesh;
}

void Chunk::CreateGLBuffers() {
	meshCreateGLBuffers( mesh );
	meshCreateGLBuffers( transparentMesh );
}

void Chunk::UpdateGLBuffers() {
	meshUpdateBuffer( mesh );
	meshUpdateBuffer( transparentMesh );
}

void Chunk::DeleteGLBuffers() {
	meshDeleteBuffers( mesh );
	meshDeleteBuffers( transparentMesh );
}
