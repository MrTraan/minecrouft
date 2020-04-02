#include "Camera.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "ngLib/nglib.h"
#include <ChunkManager.hpp>
#include <Guizmo.hpp>
#include <LZ4.h>
#include <algorithm>
#include <chrono>
#include <concurrentqueue.h>
#include <imgui/imgui.h>
#include <stdio.h>
#include <tracy/Tracy.hpp>

#include <Game.h>

using MPMCChunkQueue = moodycamel::ConcurrentQueue< Chunk * >;
MPMCChunkQueue buildingQueueIn;
MPMCChunkQueue buildingQueueOut;

std::atomic_bool buildersShouldRun( true );

static void builderThreadRoutine( ChunkManager * manager ) {
	using namespace std::chrono_literals;
	char * readBuffer = new char[ sizeof( Chunk::cubes ) ];
	while ( buildersShouldRun.load() == true ) {
		Chunk * chunk;
		bool    found = buildingQueueIn.try_dequeue( chunk );
		if ( !found ) {
			std::this_thread::sleep_for( 16ms );
		} else {
			// Check if chunk is in save file
			// Otherwise, generate from heightmap and save to file
			if ( manager->chunksMetaInfo.find( chunk->position ) != manager->chunksMetaInfo.end() ) {
				// load from file
				ChunkManager::MetaChunkInfo info = manager->chunksMetaInfo[ chunk->position ];
				fseek( manager->chunkDataFp, info.binaryOffset, SEEK_SET );
				fread( readBuffer, info.binarySize, 1, manager->chunkDataFp );
				auto ret = LZ4_decompress_safe( readBuffer, ( char * )( &chunk->cubes ), info.binarySize,
				                                sizeof( Chunk::cubes ) );
				ng_assert( ret == sizeof( Chunk::cubes ) );
			} else {
				manager->heightMap.SetupChunk( chunk );
				// A new chunk has been generated, we should write it to save file
			}
			chunkCreateGeometry( chunk );
			buildingQueueOut.enqueue( chunk );
		}
	}
	delete[] readBuffer;
}

ChunkCoordinates WorldToChunkPosition( const glm::vec3 & pos ) {
	return createChunkCoordinates( ( s32 )pos.x / CHUNK_SIZE, ( s32 )pos.z / CHUNK_SIZE );
}

glm::vec3 ChunkToWorldPosition( const glm::i32vec2 & pos ) {
	return glm::vec3( pos.x * CHUNK_SIZE, 0, pos.x * CHUNK_SIZE );
}

glm::vec3 ChunkToWorldPosition( ChunkCoordinates pos ) {
	return glm::vec3( ( s32 )getXCoord( pos ) * CHUNK_SIZE, 0, ( s32 )getZCoord( pos ) * CHUNK_SIZE );
}

void ChunkManager::Init( const glm::vec3 & playerPos ) {
	ZoneScoped;
	shader.CompileFromPath( "./resources/shaders/vertex.glsl", "./resources/shaders/fragment.glsl" );
	textureAtlas = loadTextureAtlas( "./resources/blocks_pixel_perfect.png", 7, 3 );

	for ( int i = 0; i < NUM_MANAGER_THREADS; i++ )
		builderRoutineThreads[ i ] = std::thread( builderThreadRoutine, this );

	LoadMetaDataFile( "world.meta" );
	chunkDataFp = fopen( "world.save", "a+b" );
	ng_assert( chunkDataFp != nullptr );
	if ( chunkDataFp == nullptr ) {
		ng::Errorf( "Could not open save file %s\n", "world.save" );
	}

	ChunkCoordinates position = WorldToChunkPosition( playerPos );
	CreateChunksAroundPlayer( position );

	// Flush building queue while player chunk is not ready
	Chunk * builtChunk = nullptr;
	bool    playerChunkCreated = false;
	while ( playerChunkCreated == false ) {
		while ( buildingQueueOut.try_dequeue( builtChunk ) ) {
			chunks[ builtChunk->position ] = builtChunk;
			builtChunk->CreateGLBuffers();
			if ( builtChunk->position == position ) {
				playerChunkCreated = true;
				break;
			}
		}
		using namespace std::chrono_literals;
		std::this_thread::sleep_for( 10ms );
	}
}

void ChunkManager::Shutdown() {
	ZoneScoped;
	buildersShouldRun.store( false );
	for ( int i = 0; i < NUM_MANAGER_THREADS; i++ )
		builderRoutineThreads[ i ].join();
	for ( auto & e : chunks ) {
		e.second->DeleteGLBuffers();
		chunkDestroy( e.second );
		delete e.second;
	}
	while ( poolHead != nullptr ) {
		chunkDestroy( poolHead );
		auto newHead = poolHead->poolNextItem;
		delete poolHead;
		poolHead = newHead;
	}
}

inline bool ChunkManager::ChunkIsLoaded( ChunkCoordinates pos ) { return chunks.find( pos ) != chunks.end(); }

inline bool ChunkManager::ChunkIsLoaded( u16 x, u16 y ) {
	return chunks.find( createChunkCoordinates( x, y ) ) != chunks.end();
}

Chunk * ChunkManager::GetChunkAt( ChunkCoordinates coord ) {
	if ( chunks.find( coord ) != chunks.end() ) {
		return chunks[ coord ];
	}
	return nullptr;
}

bool ChunkManager::PushChunkToProducer( ChunkCoordinates coord ) {
	auto chunk = popChunkFromPool();
	chunk->position = coord;
	chunk->biome = eBiome::MOUNTAIN;
	return buildingQueueIn.enqueue( chunk );
}

void ChunkManager::Update( const glm::vec3 & playerPos ) {
	ZoneScoped;
	ChunkCoordinates position = WorldToChunkPosition( playerPos );

	// Flush building queue
	Chunk * builtChunk = nullptr;
	while ( buildingQueueOut.try_dequeue( builtChunk ) ) {
		if ( !ChunkIsLoaded( builtChunk->position ) ) {
			chunks[ builtChunk->position ] = builtChunk;
			builtChunk->CreateGLBuffers();
		} else {
			// Race condition: the element was built twice
			pushChunkToPool( builtChunk );
		}
	}

	if ( position == lastPosition )
		return;
	lastPosition = position;

	// Remove chunks outside of unload radius
	for ( auto it = std::begin( chunks ); it != std::end( chunks ); ) {
		auto cpos = it->second->position;
		if ( abs( getXCoord( position ) - getXCoord( cpos ) ) > chunkUnloadRadius ||
		     abs( getZCoord( position ) - getZCoord( cpos ) ) > chunkUnloadRadius ) {
			it->second->DeleteGLBuffers();
			pushChunkToPool( it->second );
			it = chunks.erase( it );
		} else {
			++it;
		}
	}

	CreateChunksAroundPlayer( position );
}

void ChunkManager::CreateChunksAroundPlayer( ChunkCoordinates chunkPosition ) {
	ZoneScoped;
	glm::u16vec2 cursor( 0, 0 );

	for ( cursor.x = MAX( 0, getXCoord( chunkPosition ) - chunkLoadRadius );
	      cursor.x <= MIN( CHUNK_MAX_X, getXCoord( chunkPosition ) + chunkLoadRadius ); cursor.x++ ) {
		for ( cursor.y = MAX( 0, getZCoord( chunkPosition ) - chunkLoadRadius );
		      cursor.y <= MIN( CHUNK_MAX_Z, getZCoord( chunkPosition ) + chunkLoadRadius ); cursor.y++ ) {
			ChunkCoordinates pos = createChunkCoordinates( cursor.x, cursor.y );
			if ( !ChunkIsLoaded( pos ) ) {
				PushChunkToProducer( pos );
			}
		}
	}
}

bool ChunkManager::LoadMetaDataFile( const char * metaFilePath ) {
	chunksMetaInfo.clear();
	FILE * metaFp = fopen( metaFilePath, "rb" );
	//ng_assert( metaFp != nullptr );
	if ( metaFp == nullptr ) {
		ng::Errorf( "Could not open meta save file %s\n", metaFilePath );
		return false;
	}

	MetaChunkInfo info;
	while ( fread( &info, sizeof( MetaChunkInfo ), 1, metaFp ) > 0 ) {
		chunksMetaInfo[ info.coord ] = info;
	}

	ng::Printf( "%llu meta info read\n", chunksMetaInfo.size() );

	fclose( metaFp );
	return true;
}

bool ChunkManager::SaveWorldToFile( const char * path, const char * metaFilePath ) {
	ZoneScoped;
	FILE * dataFp = fopen( path, "wb" );
	ng_assert( dataFp != nullptr );
	if ( dataFp == nullptr ) {
		ng::Errorf( "Could not open save file %s\n", path );
		return false;
	}
	FILE * metaFp = fopen( metaFilePath, "wb" );
	ng_assert( metaFp != nullptr );
	if ( metaFp == nullptr ) {
		ng::Errorf( "Could not open save file %s\n", metaFilePath );
		fclose( dataFp );
		return false;
	}

	size_t chunkDataSize = sizeof( Chunk::cubes );
	size_t chunkCompressedBoundSize = LZ4_compressBound( chunkDataSize );
	char * compressedData = new char[ chunkCompressedBoundSize ];

	size_t writeOffset = 0;

	for ( auto it : chunks ) {
		ChunkCoordinates coord = it.first;
		Chunk *          chunk = it.second;

		MetaChunkInfo info;
		info.coord = coord;
		info.binaryOffset = writeOffset;

		int compressedBytes = LZ4_compress_default( ( char * )( &chunk->cubes ), compressedData, chunkDataSize,
		                                            chunkCompressedBoundSize );
		ng_assert( compressedBytes > 0 );

		info.binarySize = compressedBytes;

		fwrite( &info, sizeof( MetaChunkInfo ), 1, metaFp );
		fwrite( compressedData, compressedBytes, 1, dataFp );

		writeOffset += compressedBytes;
	}

	delete[] compressedData;
	fclose( dataFp );
	fclose( metaFp );
	return true;
}

bool ChunkManager::LoadWorldFromFile( const char * path, const char * metaFilePath ) {
	ZoneScoped;

	// TODO: Flush jobs before

	FILE * dataFp = fopen( path, "rb" );
	ng_assert( dataFp != nullptr );
	if ( dataFp == nullptr ) {
		ng::Errorf( "Could not open save file %s\n", path );
		return false;
	}
	FILE * metaFp = fopen( metaFilePath, "rb" );
	ng_assert( metaFp != nullptr );
	if ( metaFp == nullptr ) {
		ng::Errorf( "Could not open save file %s\n", metaFilePath );
		fclose( dataFp );
		return false;
	}

	for ( auto & e : chunks ) {
		e.second->DeleteGLBuffers();
		pushChunkToPool( e.second );
	}
	chunks.clear();

	// Prepare a buffer large enough to read uncompressed bytes
	char * readBuffer = new char[ sizeof( Chunk::cubes ) ];

	MetaChunkInfo info;
	while ( fread( &info, sizeof( MetaChunkInfo ), 1, metaFp ) > 0 ) {
		Chunk * chunk = popChunkFromPool();
		chunk->position = info.coord;
		chunk->biome = eBiome::MOUNTAIN;

		fseek( dataFp, info.binaryOffset, SEEK_SET );
		fread( readBuffer, info.binarySize, 1, dataFp );

		auto ret =
		    LZ4_decompress_safe( readBuffer, ( char * )( &chunk->cubes ), info.binarySize, sizeof( Chunk::cubes ) );
		ng_assert( ret > 0 );
		buildingQueueIn.enqueue( chunk );
	}

	delete[] readBuffer;
	fclose( dataFp );
	fclose( metaFp );

	// Flush building queue while player chunk is not ready
	ChunkCoordinates playerPosition = WorldToChunkPosition( theGame->player.position );
	Chunk *          builtChunk = nullptr;
	bool             playerChunkCreated = false;
	while ( playerChunkCreated == false ) {
		while ( buildingQueueOut.try_dequeue( builtChunk ) ) {
			chunks[ builtChunk->position ] = builtChunk;
			builtChunk->CreateGLBuffers();
			if ( builtChunk->position == playerPosition ) {
				playerChunkCreated = true;
				break;
			}
		}
		using namespace std::chrono_literals;
		std::this_thread::sleep_for( 10ms );
	}
	return true;
}

void ChunkManager::Draw( const Camera & camera ) {
	ZoneScoped;
	shader.Use();
	int viewLoc = glGetUniformLocation( shader.ID, "view" );
	glUniformMatrix4fv( viewLoc, 1, GL_FALSE, glm::value_ptr( camera.viewMatrix ) );
	int projLoc = glGetUniformLocation( shader.ID, "projection" );
	glUniformMatrix4fv( projLoc, 1, GL_FALSE, glm::value_ptr( camera.projMatrix ) );
	int modelLoc = glGetUniformLocation( shader.ID, "model" );

	bindTextureAtlas( textureAtlas );

	static glm::vec3 sizeOffset = glm::vec3( ( float )CHUNK_SIZE, ( float )CHUNK_HEIGHT, ( float )CHUNK_SIZE );
	Aabb             bounds;

	drawCallsLastFrame = 0;

	if ( debugDrawOpaque == true ) {
		for ( auto & it : chunks ) {
			Chunk * chunk = it.second;

			ng_assert( chunk != nullptr );
			auto worldPosition = ChunkToWorldPosition( chunk->position );
			bounds.min = worldPosition;
			bounds.max = worldPosition + sizeOffset;
			// Check if any of eight corners of the chunk is in sight
			if ( camera.frustrum.IsCubeIn( bounds ) ) {
				auto translationMatrix = glm::translate( glm::mat4( 1.0f ), worldPosition );
				glUniformMatrix4fv( modelLoc, 1, GL_FALSE, glm::value_ptr( translationMatrix ) );
				meshDraw( chunk->mesh );
				drawCallsLastFrame++;
			}
		}
	}

	if ( debugDrawTransparent == true ) {
		for ( auto & it : chunks ) {
			Chunk * chunk = it.second;
			ng_assert( chunk != nullptr );
			if ( chunk->transparentMesh->facesBuilt == 0 ) {
				continue;
			}
			auto worldPosition = ChunkToWorldPosition( chunk->position );
			bounds.min = worldPosition;
			bounds.max = worldPosition + sizeOffset;
			// Check if any of eight corners of the chunk is in sight
			if ( camera.frustrum.IsCubeIn( bounds ) ) {
				auto translationMatrix = glm::translate( glm::mat4( 1.0f ), worldPosition );
				glUniformMatrix4fv( modelLoc, 1, GL_FALSE, glm::value_ptr( translationMatrix ) );
				meshDraw( chunk->transparentMesh );
			}
		}
	}
}

Chunk * ChunkManager::popChunkFromPool() {
	if ( poolHead == nullptr ) {
		return preallocateChunk();
	}
	auto item = poolHead;
	poolHead = poolHead->poolNextItem;
	item->poolNextItem = nullptr;
	return item;
}

void ChunkManager::pushChunkToPool( Chunk * item ) {
	if ( poolHead == nullptr ) {
		poolHead = item;
	} else {
		item->poolNextItem = poolHead;
		poolHead = item;
	}
}

void ChunkManager::DebugDraw() {
	ImGui::Text( "Chunks loaded: %lu\n", chunks.size() );
	u32 poolSize = 0;
	for ( auto it = poolHead; it != nullptr; it = it->poolNextItem )
		poolSize++;
	ImGui::Text( "Chunks pooled: %d\n", poolSize );

	static int newChunkLoadRadius = chunkLoadRadius;
	static int newChunkUnloadRadius = chunkUnloadRadius;
	ImGui::SliderInt( "Chunk load radius", &newChunkLoadRadius, 1, 32 );
	ImGui::SliderInt( "Chunk unload radius", &newChunkUnloadRadius, 1, 32 );

	bool forceUpdate = false;
	if ( ImGui::Button( "Apply" ) ) {
		ZoneScopedN( "ChunkLoadRadiusUpdate" );
		chunkLoadRadius = newChunkLoadRadius;
		chunkUnloadRadius = newChunkUnloadRadius;
		CreateChunksAroundPlayer( WorldToChunkPosition( theGame->player.position ) );
	}
	ImGui::Text( "Render distance: %d cubes\n", chunkLoadRadius * CHUNK_SIZE );
	ImGui::Text( "Chunks drawn: %u / %lu\n", drawCallsLastFrame, chunks.size() );

	static bool drawChunkBoundaries = false;
	ImGui::Checkbox( "Draw chunk boundaries", &drawChunkBoundaries );
	if ( drawChunkBoundaries ) {
		ChunkCoordinates playerChunkCoord = WorldToChunkPosition( theGame->player.position );
		glm::vec3        playerChunkPosition = ChunkToWorldPosition( playerChunkCoord );

		// Draw current chunk horizontal lines
		for ( int i = 0; i < CHUNK_HEIGHT; i += CHUNK_HEIGHT / 16 ) {
			Guizmo::Line( playerChunkPosition + glm::vec3( 0.0f, i, 0.0f ),
			              playerChunkPosition + glm::vec3( CHUNK_SIZE, i, 0.0f ), Guizmo::colYellow );
			Guizmo::Line( playerChunkPosition + glm::vec3( 0.0f, i, 0.0f ),
			              playerChunkPosition + glm::vec3( 0.0f, i, CHUNK_SIZE ), Guizmo::colYellow );
			Guizmo::Line( playerChunkPosition + glm::vec3( CHUNK_SIZE, i, 0.0f ),
			              playerChunkPosition + glm::vec3( CHUNK_SIZE, i, CHUNK_SIZE ), Guizmo::colYellow );
			Guizmo::Line( playerChunkPosition + glm::vec3( 0.0f, i, CHUNK_SIZE ),
			              playerChunkPosition + glm::vec3( CHUNK_SIZE, i, CHUNK_SIZE ), Guizmo::colYellow );
		}

		// Draw current chunk and neihgbors vertical lines
		for ( int x = -1; x <= 2; x++ ) {
			for ( int z = -1; z <= 2; z++ ) {
				Guizmo::Line( playerChunkPosition + glm::vec3( x * CHUNK_SIZE, 0.0f, z * CHUNK_SIZE ),
				              playerChunkPosition + glm::vec3( x * CHUNK_SIZE, CHUNK_HEIGHT, z * CHUNK_SIZE ),
				              ( x == 0 || x == 1 ) && ( z == 0 || z == 1 ) ? Guizmo::colWhite : Guizmo::colRed );
			}
		}
	}

	ImGui::Checkbox( "Draw opaque cubes", &debugDrawOpaque );
	ImGui::SameLine();
	ImGui::Checkbox( "Draw transparent cubes", &debugDrawTransparent );

	if ( ImGui::Button( "Test save" ) ) {
		SaveWorldToFile( "world.save", "world.meta" );
	}

	if ( ImGui::Button( "Test load" ) ) {
		LoadWorldFromFile( "world.save", "world.meta" );
	}
}
