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

static void pushFace( Chunk *    chunk,
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
	chunk->facesBuilt = 0;
	chunk->mesh->IndicesCount = 0;
	chunk->mesh->VerticesCount = 0;

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

							if ( d == 0 )
								pushFace( chunk, p4, p1, p2, p3, h, w, dir, reverse, mask[ n ] );
							else
								pushFace( chunk, p1, p2, p3, p4, w, h, dir, reverse, mask[ n ] );

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
	// if ( chunk->facesAllocated > chunk->facesBuilt * 2 ) j
	//	ZoneScopedN( "ShrunkingFacesAllocated" );
	//	chunk->mesh->Indices = ( u32 * )realloc( chunk->mesh->Indices, sizeof( u32 ) * 6 * chunk->facesBuilt );
	//	ng_assert( chunk->mesh->Indices != NULL );
	//	chunk->mesh->Vertices = ( Vertex * )realloc( chunk->mesh->Vertices, sizeof( Vertex ) * 4 * chunk->facesBuilt );
	//	ng_assert( chunk->mesh->Vertices != NULL );
	//	chunk->facesAllocated = chunk->facesBuilt;
	//}
}

void pushFace( Chunk *    chunk,
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
	case eBlockType::ROCK:
		uvModifier = 1;
		break;
	case eBlockType::SNOW:
		uvModifier = 2;
		break;
	case eBlockType::GRASS:
		uvModifier = 3;
		break;
	default:
		uvModifier = 0;
		break;
	}

	while ( chunk->facesAllocated <= chunk->facesBuilt ) {
		ZoneScopedN( "GrowingFacesAllocated" );
		auto grownIndices = ( u32 * )ng_alloc( sizeof( u32 ) * 6 * ( FACES_BATCH_ALLOC + chunk->facesAllocated ) );
		ng_assert( grownIndices != nullptr );
		auto grownVertices =
		    ( Vertex * )ng_alloc( sizeof( Vertex ) * 4 * ( FACES_BATCH_ALLOC + chunk->facesAllocated ) );
		ng_assert( grownVertices != nullptr );
		memcpy( grownIndices, chunk->mesh->Indices, sizeof( u32 ) * 6 * chunk->facesAllocated );
		memcpy( grownVertices, chunk->mesh->Vertices, sizeof( Vertex ) * 4 * chunk->facesAllocated );
		ng_free( chunk->mesh->Indices );
		ng_free( chunk->mesh->Vertices );
		chunk->mesh->Indices = grownIndices;
		chunk->mesh->Vertices = grownVertices;
		chunk->facesAllocated += FACES_BATCH_ALLOC;
	}

	// Vertex index
	u32 vi = 4 * chunk->facesBuilt;
	// Indices index
	u32 ii = 6 * chunk->facesBuilt;

	chunk->facesBuilt++;

	chunk->mesh->IndicesCount += 6;
	if ( reverse ) {
		chunk->mesh->Indices[ ii + 0 ] = vi + 1;
		chunk->mesh->Indices[ ii + 1 ] = vi + 0;
		chunk->mesh->Indices[ ii + 2 ] = vi + 3;
		chunk->mesh->Indices[ ii + 3 ] = vi + 3;
		chunk->mesh->Indices[ ii + 4 ] = vi + 2;
		chunk->mesh->Indices[ ii + 5 ] = vi + 1;
	} else {
		chunk->mesh->Indices[ ii + 0 ] = vi + 1;
		chunk->mesh->Indices[ ii + 1 ] = vi + 2;
		chunk->mesh->Indices[ ii + 2 ] = vi + 3;
		chunk->mesh->Indices[ ii + 3 ] = vi + 3;
		chunk->mesh->Indices[ ii + 4 ] = vi + 0;
		chunk->mesh->Indices[ ii + 5 ] = vi + 1;
	}

	chunk->mesh->VerticesCount += 4;
	Vertex * v = chunk->mesh->Vertices;

	v[ vi + 0 ].TexCoords[ 0 ] = 0.0f;
	v[ vi + 0 ].TexCoords[ 1 ] = 0.0f;
	v[ vi + 1 ].TexCoords[ 0 ] = width;
	v[ vi + 1 ].TexCoords[ 1 ] = 0.0f;
	v[ vi + 2 ].TexCoords[ 0 ] = width;
	v[ vi + 2 ].TexCoords[ 1 ] = height;
	v[ vi + 3 ].TexCoords[ 0 ] = 0.0f;
	v[ vi + 3 ].TexCoords[ 1 ] = height;

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
			v[ vi + i ].TexIndex = float( uvModifier * 4 );
		}
		break;
	case eDirection::BOTTOM:
		for ( int i = 0; i < 4; i++ ) {
			v[ vi + i ].TexIndex = float( uvModifier * 4 + 3 );
		}
		break;
	case eDirection::SOUTH:
	case eDirection::EAST:
	case eDirection::WEST:
	case eDirection::NORTH:
		for ( int i = 0; i < 4; i++ ) {
			v[ vi + i ].TexIndex = float( uvModifier * 4 + 1 );
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
	chunk->facesAllocated = FACES_INITIAL_ALLOC;
	chunk->facesBuilt = 0;
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
}

#define CUBES_TEXTURE_PATH "./resources/blocks_pixel_perfect.png"

void chunkDraw( Chunk * chunk ) {
	static TextureAtlas ta = loadTextureAtlas( CUBES_TEXTURE_PATH, 4, 4 );

	bindTextureAtlas( ta );
	meshDraw( chunk->mesh );
}
