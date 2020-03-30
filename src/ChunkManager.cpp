#include "Camera.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "ngLib/nglib.h"
#include <ChunkManager.hpp>
#include <algorithm>
#include <chrono>
#include <concurrentqueue.h>
#include <imgui/imgui.h>
#include <tracy/Tracy.hpp>

#include <Game.h>

using MPMCChunkQueue = moodycamel::ConcurrentQueue< Chunk * >;
MPMCChunkQueue buildingQueueIn;
MPMCChunkQueue buildingQueueOut;

std::atomic_bool buildersShouldRun( true );

static void builderThreadRoutine( ChunkManager * manager ) {
	using namespace std::chrono_literals;
	while ( buildersShouldRun.load() == true ) {
		Chunk * chunk;
		bool    found = buildingQueueIn.try_dequeue( chunk );
		if ( !found ) {
			std::this_thread::sleep_for( 16ms );
		} else {
			manager->heightMap.SetupChunk( chunk );
			chunkCreateGeometry( chunk );
			buildingQueueOut.enqueue( chunk );
		}
	}
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

	for ( int i = 0; i < NUM_MANAGER_THREADS; i++ )
		builderRoutineThreads[ i ] = std::thread( builderThreadRoutine, this );

	ChunkCoordinates position = WorldToChunkPosition( playerPos );
	CreateChunksAroundPlayer( position );

	// Flush building queue while player chunk is not ready
	Chunk * builtChunk = nullptr;
	bool    playerChunkCreated = false;
	while ( playerChunkCreated == false ) {
		while ( buildingQueueOut.try_dequeue( builtChunk ) ) {
			chunks[ builtChunk->position ] = builtChunk;
			meshCreateGLBuffers( builtChunk->mesh );
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
		meshDeleteBuffers( e.second->mesh );
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
			meshCreateGLBuffers( builtChunk->mesh );
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
			meshDeleteBuffers( it->second->mesh );
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

void ChunkManager::Draw( const Camera & camera ) {
	ZoneScoped;
	shader.Use();
	int viewLoc = glGetUniformLocation( shader.ID, "view" );
	glUniformMatrix4fv( viewLoc, 1, GL_FALSE, glm::value_ptr( camera.viewMatrix ) );
	int projLoc = glGetUniformLocation( shader.ID, "projection" );
	glUniformMatrix4fv( projLoc, 1, GL_FALSE, glm::value_ptr( camera.projMatrix ) );
	int modelLoc = glGetUniformLocation( shader.ID, "model" );

	static glm::vec3 sizeOffset = glm::vec3( ( float )CHUNK_SIZE, ( float )CHUNK_HEIGHT, ( float )CHUNK_SIZE );
	Aabb             bounds;

	drawCallsLastFrame = 0;

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
			chunkDraw( chunk );
			drawCallsLastFrame++;
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
}
