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

constexpr glm::u32vec3 waterVerticesOffset( 0, -1, 0 );

void chunkCreateGeometry( Chunk * chunk ) {
	ZoneScoped;
	static thread_local eBlockType mask[ CHUNK_SIZE * CHUNK_HEIGHT ];
	chunk->mesh->verticesCount = 0;
	chunk->transparentMesh->verticesCount = 0;

	int dims[ 3 ] = {CHUNK_SIZE, CHUNK_HEIGHT, CHUNK_SIZE};

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
							glm::u32vec3 p1( indices[ 0 ], indices[ 1 ], indices[ 2 ] );
							glm::u32vec3 p2( indices[ 0 ] + du[ 0 ], indices[ 1 ] + du[ 1 ], indices[ 2 ] + du[ 2 ] );
							glm::u32vec3 p3( indices[ 0 ] + du[ 0 ] + dv[ 0 ], indices[ 1 ] + du[ 1 ] + dv[ 1 ],
							                 indices[ 2 ] + du[ 2 ] + dv[ 2 ] );
							glm::u32vec3 p4( indices[ 0 ] + dv[ 0 ], indices[ 1 ] + dv[ 1 ], indices[ 2 ] + dv[ 2 ] );
							p1 = p1 * 10u;
							p2 = p2 * 10u;
							p3 = p3 * 10u;
							p4 = p4 * 10u;

							if ( mask[ n ] == eBlockType::WATER ) {
								// ONLY DRAW TOP SURFACE
								if ( dir == eDirection::TOP ) {
									chunkPushFace( chunk->transparentMesh, p4 + waterVerticesOffset,
									               p1 + waterVerticesOffset, p2 + waterVerticesOffset,
									               p3 + waterVerticesOffset, h, w, dir, mask[ n ] );
								}
							} else {
								if ( d == 0 ) {
									chunkPushFace( chunk->mesh, p4, p1, p2, p3, h, w, dir, mask[ n ] );
								} else {
									chunkPushFace( chunk->mesh, p1, p2, p3, p4, w, h, dir, mask[ n ] );
								}
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

void chunkPushFace( VoxelMesh *  mesh,
                    glm::u32vec3 a,
                    glm::u32vec3 b,
                    glm::u32vec3 c,
                    glm::u32vec3 d,
                    int          width,
                    int          height,
                    eDirection   direction,
                    eBlockType   type ) {
	if ( mesh->verticesCount + 6 > mesh->verticesAllocated ) {
		ZoneScopedN( "GrowingFacesAllocated" );
		auto grownVertices = ( VoxelVertex * )ng_alloc( sizeof( VoxelVertex ) *
		                                                ( CHUNK_VERTICES_BATCH_ALLOC + mesh->verticesAllocated ) );
		ng_assert( grownVertices != nullptr );
		memcpy( grownVertices, mesh->Vertices, sizeof( VoxelVertex ) * mesh->verticesCount );
		ng_free( mesh->Vertices );
		mesh->Vertices = grownVertices;
		mesh->verticesAllocated = CHUNK_VERTICES_BATCH_ALLOC + mesh->verticesAllocated;
	}

	VoxelVertex * v = mesh->Vertices;
	// Vertex index
	u32 vi = mesh->verticesCount;
	mesh->verticesCount += 6;

	if ( direction == eDirection::WEST || direction == eDirection::BOTTOM || direction == eDirection::SOUTH ) {
		v[ vi + 0 ].SetPosition( a );
		v[ vi + 1 ].SetPosition( c );
		v[ vi + 2 ].SetPosition( b );
		v[ vi + 3 ].SetPosition( a );
		v[ vi + 4 ].SetPosition( d );
		v[ vi + 5 ].SetPosition( c );
		v[ vi + 0 ].SetTexture( 0, height );
		v[ vi + 1 ].SetTexture( width, 0 );
		v[ vi + 2 ].SetTexture( width, height );
		v[ vi + 3 ].SetTexture( 0, height );
		v[ vi + 4 ].SetTexture( 0, 0 );
		v[ vi + 5 ].SetTexture( width, 0 );
	} else {
		v[ vi + 0 ].SetPosition( a );
		v[ vi + 1 ].SetPosition( b );
		v[ vi + 2 ].SetPosition( c );
		v[ vi + 3 ].SetPosition( a );
		v[ vi + 4 ].SetPosition( c );
		v[ vi + 5 ].SetPosition( d );
		v[ vi + 0 ].SetTexture( 0, height );
		v[ vi + 1 ].SetTexture( width, height );
		v[ vi + 2 ].SetTexture( width, 0 );
		v[ vi + 3 ].SetTexture( 0, height );
		v[ vi + 4 ].SetTexture( width, 0 );
		v[ vi + 5 ].SetTexture( 0, 0 );
	}

	u8 top, bottom, side;
	BlockGetTextureIndices( type, top, bottom, side );

	switch ( direction ) {
	case eDirection::TOP:
		for ( int i = 0; i < 6; i++ ) {
			v[ vi + i ].texIndex = top;
		}
		break;
	case eDirection::BOTTOM:
		for ( int i = 0; i < 6; i++ ) {
			v[ vi + i ].texIndex = bottom;
		}
		break;
	case eDirection::SOUTH:
	case eDirection::EAST:
	case eDirection::WEST:
	case eDirection::NORTH:
		for ( int i = 0; i < 6; i++ ) {
			v[ vi + i ].texIndex = side;
		}
		break;
	}
}

Chunk * preallocateChunk() {
	ZoneScoped;
	Chunk * chunk = new Chunk();
	chunk->mesh = new VoxelMesh();
	chunk->mesh->Vertices = ( VoxelVertex * )ng_alloc( sizeof( VoxelVertex ) * CHUNK_VERTICES_INITIAL_ALLOC );
	ng_assert( chunk->mesh->Vertices != NULL );
	chunk->mesh->verticesCount = 0;
	chunk->mesh->verticesAllocated = CHUNK_VERTICES_INITIAL_ALLOC;

	chunk->transparentMesh = new VoxelMesh();
	chunk->transparentMesh->Vertices = ( VoxelVertex * )ng_alloc( sizeof( VoxelVertex ) * 6 * 8 );
	ng_assert( chunk->transparentMesh->Vertices != NULL );
	chunk->transparentMesh->verticesCount = 0;
	chunk->transparentMesh->verticesAllocated = 6 * 8;

	return chunk;
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

void Chunk::Destroy() {
	if ( mesh != nullptr ) {
		ng_free( mesh->Vertices );
	}
	delete mesh;
	if ( transparentMesh != nullptr ) {
		ng_free( transparentMesh->Vertices );
	}
	delete transparentMesh;
}
