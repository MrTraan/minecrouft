#include "Camera.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "ngLib/nglib.h"
#include <ChunkManager.hpp>
#include <algorithm>
#include <chrono>
#include <concurrentqueue.h>
#include <imgui/imgui.h>
#include <tracy/Tracy.hpp>

using MPMCChunkQueue = moodycamel::ConcurrentQueue< Chunk * >;
MPMCChunkQueue buildingQueueIn;
MPMCChunkQueue buildingQueueOut;

static void builderThreadRoutine( ChunkManager * manager, int index ) {
	using namespace std::chrono_literals;
	std::unique_lock< std::mutex > ulock( manager->ucMutex, std::defer_lock );

	while ( manager->ThreadShouldRun ) {
		Chunk * chunk;
		bool    found = buildingQueueIn.try_dequeue( chunk );
		if ( !found ) {
			std::this_thread::sleep_for( 16ms );
		} else {
			chunkCreateGeometry( chunk, &( manager->heightMap ) );
			buildingQueueOut.enqueue( chunk );
		}
	}
}

glm::i32vec2 WorldToChunkPosition( const glm::vec3 & pos ) {
	glm::i32vec2 chunkPosition( ( s32 )pos.x / CHUNK_SIZE, ( s32 )pos.z / CHUNK_SIZE );

	if ( pos.x < 0 )
		chunkPosition.x--;
	if ( pos.z < 0 )
		chunkPosition.y--;

	return chunkPosition;
}

glm::vec3 ChunkToWorldPosition( const glm::i32vec2 & pos ) {
	return glm::vec3( pos.x * CHUNK_SIZE, 0, pos.x * CHUNK_SIZE );
}

glm::vec3 ChunkToWorldPosition( ChunkCoordinates pos ) {
	return glm::vec3( ( s32 )getXCoord( pos ) * CHUNK_SIZE, 0, ( s32 )getZCoord( pos ) * CHUNK_SIZE );
}

ChunkManager::ChunkManager( glm::vec3 playerPos, Frustrum * frustrum )
    : frustrum( frustrum ), shader( "./resources/shaders/vertex.glsl", "./resources/shaders/fragment.glsl" ) {
	ThreadShouldRun = true;
	playerPos.y = 0.0f;
	auto chunkPosition = WorldToChunkPosition( playerPos );
	lastPosition = chunkPosition;

	glm::u16vec2 cursor( 0, 0 );

	for ( cursor.x = MAX( 0, chunkPosition.x - chunkLoadRadius ); cursor.x <= chunkPosition.x + chunkLoadRadius;
	      cursor.x++ ) {
		for ( cursor.y = MAX( 0, chunkPosition.y - chunkLoadRadius ); cursor.y <= chunkPosition.y + chunkLoadRadius;
		      cursor.y++ ) {
			ChunkCoordinates pos = createChunkCoordinates( cursor.x, cursor.y );
			auto             chunk = popChunkFromPool();
			chunk->position = pos;
			chunk->biome = eBiome::MOUNTAIN;
			chunkCreateGeometry( chunk, &heightMap );
			chunks[ pos ] = chunk;
		}
	}

	for ( int i = 0; i < 50; i++ )
		pushChunkToPool( preallocateChunk() );

	for ( int i = 0; i < NUM_MANAGER_THREADS; i++ )
		builderRoutineThreads[ i ] = std::thread( builderThreadRoutine, this, i );
}

ChunkManager::~ChunkManager() {
	ThreadShouldRun = false;
	ucMutex.lock();
	updateCondition.notify_all();
	ucMutex.unlock();
	for ( int i = 0; i < NUM_MANAGER_THREADS; i++ )
		builderRoutineThreads[ i ].join();
	for ( auto & e : chunks ) {
		meshDeleteBuffers( e.second->mesh );
		chunkDestroy( e.second );
		delete e.second;
	}
	while ( poolHead != nullptr ) {
		chunkDestroy( poolHead );
		poolHead = poolHead->poolNextItem;
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

void ChunkManager::Update( glm::vec3 playerPos ) {
	ZoneScoped;
	glm::i32vec2 position = WorldToChunkPosition( playerPos );
	s32          deltaX = position.x - lastPosition.x;
	s32          deltaY = position.y - lastPosition.y;

	ImGui::Text( "Chunk Position: %d %d\n", position.x, position.y );
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
		for ( u16 x = position.x - newChunkLoadRadius; x <= position.x + newChunkLoadRadius; x++ ) {
			for ( u16 y = position.y - newChunkLoadRadius; y <= position.y + newChunkLoadRadius; y++ ) {
				if ( !ChunkIsLoaded( x, y ) )
					PushChunkToProducer( createChunkCoordinates( x, y ) );
			}
		}
		chunkLoadRadius = newChunkLoadRadius;
		chunkUnloadRadius = newChunkUnloadRadius;
		forceUpdate = true;
	}
	ImGui::Text( "Render distance: %d cubes\n", chunkLoadRadius * CHUNK_SIZE );

	// Flush building queue
	Chunk * builtChunk = nullptr;
	while ( buildingQueueOut.try_dequeue( builtChunk ) ) {
		if ( !ChunkIsLoaded( builtChunk->position ) ) {
			chunks[ builtChunk->position ] = builtChunk;
		} else {
			// Race condition: the element was built twice
			if ( builtChunk->mesh->isBound ) {
				meshDeleteBuffers( builtChunk->mesh );
			}
			pushChunkToPool( builtChunk );
		}
	}

	if ( position == lastPosition && !forceUpdate )
		return;

	if ( deltaX != 0 || deltaY != 0 || forceUpdate ) {
		for ( auto it = std::begin( chunks ); it != std::end( chunks ); ) {
			auto cpos = it->second->position;
			if ( abs( position.x - getXCoord( cpos ) ) > chunkUnloadRadius ||
			     abs( position.y - getZCoord( cpos ) ) > chunkUnloadRadius ) {
				if ( it->second->mesh->isBound )
					meshDeleteBuffers( it->second->mesh );
				pushChunkToPool( it->second );
				it = chunks.erase( it );
			} else
				++it;
		}
	}

	if ( deltaX < 0 ) {
		for ( u16 x = position.x - chunkLoadRadius; x < lastPosition.x - chunkLoadRadius; x++ )
			for ( u16 y = position.y - chunkLoadRadius; y <= position.y + chunkLoadRadius; y++ )
				if ( !ChunkIsLoaded( x, y ) )
					PushChunkToProducer( createChunkCoordinates( x, y ) );
	}

	if ( deltaX > 0 ) {
		for ( u16 x = position.x + chunkLoadRadius; x > lastPosition.x + chunkLoadRadius; x-- )
			for ( u16 y = position.y - chunkLoadRadius; y <= position.y + chunkLoadRadius; y++ )
				if ( !ChunkIsLoaded( x, y ) )
					PushChunkToProducer( createChunkCoordinates( x, y ) );
	}

	if ( deltaY < 0 ) {
		for ( u16 y = position.y - chunkLoadRadius; y < lastPosition.y - chunkLoadRadius; y++ )
			for ( u16 x = position.x - chunkLoadRadius; x <= position.x + chunkLoadRadius; x++ )
				if ( !ChunkIsLoaded( x, y ) )
					PushChunkToProducer( createChunkCoordinates( x, y ) );
	}

	if ( deltaY > 0 ) {
		for ( u16 y = position.y + chunkLoadRadius; y > lastPosition.y + chunkLoadRadius; y-- )
			for ( u16 x = position.x - chunkLoadRadius; x <= position.x + chunkLoadRadius; x++ )
				if ( !ChunkIsLoaded( x, y ) )
					PushChunkToProducer( createChunkCoordinates( x, y ) );
	}

	lastPosition = position;
}

void ChunkManager::Draw( const Camera & camera ) {
	ZoneScoped;
	shader.Use();
	int viewLoc = glGetUniformLocation( shader.ID, "view" );
	glUniformMatrix4fv( viewLoc, 1, GL_FALSE, glm::value_ptr( camera.GetViewMatrix() ) );
	int projLoc = glGetUniformLocation( shader.ID, "projection" );
	glUniformMatrix4fv( projLoc, 1, GL_FALSE, glm::value_ptr( camera.projMatrix ) );
	int modelLoc = glGetUniformLocation( shader.ID, "model" );

	static glm::vec3 sizeOffset = glm::vec3( ( float )CHUNK_SIZE, ( float )CHUNK_HEIGHT, ( float )CHUNK_SIZE );
	Aabb             bounds;

	u32 drawCalls = 0;

	for ( auto & it : chunks ) {
		Chunk * chunk = it.second;

		ng_assert( chunk != nullptr );
		auto worldPosition = ChunkToWorldPosition( chunk->position );
		bounds.min = worldPosition;
		bounds.max = worldPosition + sizeOffset;
		// Check if any of eight corners of the chunk is in sight
		if ( frustrum->IsCubeIn( bounds ) ) {
			auto translationMatrix = glm::translate( glm::mat4( 1.0f ), worldPosition );
			glUniformMatrix4fv( modelLoc, 1, GL_FALSE, glm::value_ptr( translationMatrix ) );
			chunkDraw( chunk, shader );
			drawCalls++;
		}
	}
	ImGui::Text( "Chunks drawn: %u / %lu\n", drawCalls, chunks.size() );
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
