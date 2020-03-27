#include "Camera.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <ChunkManager.hpp>
#include <algorithm>
#include <imgui/imgui.h>
#include <tracy/Tracy.hpp>

static void builderThreadRoutine( ChunkManager * manager, int index ) {
	std::unique_lock< std::mutex > ulock( manager->ucMutex, std::defer_lock );
	eBlockType                     mask[ CHUNK_SIZE * CHUNK_HEIGHT ];

	while ( true ) {
		ulock.lock();
		manager->updateCondition.wait( ulock );
		ulock.unlock();
		if ( !manager->ThreadShouldRun )
			break;
		while ( true ) {
			if ( !manager->ThreadShouldRun )
				goto EXIT_THREAD_ROUTINE;
			manager->queueInMutex.lock();
			if ( manager->buildingQueueIn.empty() ) {
				manager->queueInMutex.unlock();
				break;
			}
			Chunk * chunk = manager->buildingQueueIn.back();
			manager->buildingQueueIn.pop_back();
			manager->queueInMutex.unlock();

			chunkCreateGeometry( chunk, &( manager->heightMap ), mask );
			manager->queueOutMutex.lock();
			manager->buildingQueueOut.push_back( chunk );
			manager->queueOutMutex.unlock();
		}
	}
EXIT_THREAD_ROUTINE:
	( void )0;
}

glm::i32vec2 ChunkManager::GetChunkPosition( glm::vec3 pos ) {
	glm::i32vec2 chunkPosition( ( s32 )pos.x / CHUNK_SIZE, ( s32 )pos.z / CHUNK_SIZE );

	if ( pos.x < 0 )
		chunkPosition.x--;
	if ( pos.z < 0 )
		chunkPosition.y--;

	return chunkPosition;
}

ChunkManager::ChunkManager( glm::vec3 playerPos, Frustrum * frustrum )
    : frustrum( frustrum ), shader( "./resources/shaders/vertex.glsl", "./resources/shaders/fragment.glsl" ) {
	ThreadShouldRun = true;
	playerPos.y = 0.0f;
	auto chunkPosition = GetChunkPosition( playerPos );
	lastPosition = chunkPosition;
	eBlockType mask[ CHUNK_SIZE * CHUNK_HEIGHT ];

	glm::u16vec2 cursor( 0, 0 );

	for ( cursor.x = chunkPosition.x - chunkLoadRadius; cursor.x <= chunkPosition.x + chunkLoadRadius; cursor.x++ ) {
		for ( cursor.y = chunkPosition.y - chunkLoadRadius; cursor.y <= chunkPosition.y + chunkLoadRadius;
		      cursor.y++ ) {
			ChunkCoordinates pos = createChunkCoordinates( cursor.x, cursor.y );
			auto             chunk = popChunkFromPool();
			chunk->position = pos;
			chunk->worldPosition =
			    glm::i32vec3( ( s32 )getXCoord( pos ) * CHUNK_SIZE, 0, ( s32 )getZCoord( pos ) * CHUNK_SIZE );
			chunk->biome = eBiome::MOUNTAIN;
			chunkCreateGeometry( chunk, &heightMap, mask );
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

void ChunkManager::Update( glm::vec3 playerPos ) {
	ZoneScoped;
	position = GetChunkPosition( playerPos );
	std::vector< ChunkCoordinates > chunksToBuild;
	s32                             deltaX = position.x - lastPosition.x;
	s32                             deltaY = position.y - lastPosition.y;

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
		for ( u16 x = position.x - newChunkLoadRadius; x <= position.x + newChunkLoadRadius; x++ ) {
			for ( u16 y = position.y - newChunkLoadRadius; y <= position.y + newChunkLoadRadius; y++ ) {
				if ( !ChunkIsLoaded( x, y ) )
					chunksToBuild.push_back( createChunkCoordinates( x, y ) );
			}
		}
		chunkLoadRadius = newChunkLoadRadius;
		chunkUnloadRadius = newChunkUnloadRadius;
		forceUpdate = true;
	}
	ImGui::Text( "Render distance: %d cubes\n", chunkLoadRadius * CHUNK_SIZE );

	// Flush building queue
	queueOutMutex.lock();
	while ( !buildingQueueOut.empty() ) {
		auto elem = buildingQueueOut.back();
		buildingQueueOut.pop_back();
		if ( !ChunkIsLoaded( elem->position ) )
			chunks[ elem->position ] = elem;
		else {
			// Race condition: the element was built twice
			if ( elem->mesh->isBound )
				meshDeleteBuffers( elem->mesh );
			pushChunkToPool( elem );
		}
	}
	queueOutMutex.unlock();

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
					chunksToBuild.push_back( createChunkCoordinates( x, y ) );
	}

	if ( deltaX > 0 ) {
		for ( u16 x = position.x + chunkLoadRadius; x > lastPosition.x + chunkLoadRadius; x-- )
			for ( u16 y = position.y - chunkLoadRadius; y <= position.y + chunkLoadRadius; y++ )
				if ( !ChunkIsLoaded( x, y ) )
					chunksToBuild.push_back( createChunkCoordinates( x, y ) );
	}

	if ( deltaY < 0 ) {
		for ( u16 y = position.y - chunkLoadRadius; y < lastPosition.y - chunkLoadRadius; y++ )
			for ( u16 x = position.x - chunkLoadRadius; x <= position.x + chunkLoadRadius; x++ )
				if ( !ChunkIsLoaded( x, y ) )
					chunksToBuild.push_back( createChunkCoordinates( x, y ) );
	}

	if ( deltaY > 0 ) {
		for ( u16 y = position.y + chunkLoadRadius; y > lastPosition.y + chunkLoadRadius; y-- )
			for ( u16 x = position.x - chunkLoadRadius; x <= position.x + chunkLoadRadius; x++ )
				if ( !ChunkIsLoaded( x, y ) )
					chunksToBuild.push_back( createChunkCoordinates( x, y ) );
	}

	if ( chunksToBuild.size() > 0 ) {
		queueInMutex.lock();

		// Flush obsolete orders
		for ( auto it = std::begin( buildingQueueIn ); it != std::end( buildingQueueIn ); ) {
			auto cpos = ( *it )->position;
			if ( abs( position.x - getXCoord( cpos ) ) > chunkUnloadRadius ||
			     abs( position.y - getZCoord( cpos ) ) > chunkUnloadRadius ) {
				pushChunkToPool( *it );
				it = buildingQueueIn.erase( it );
			} else
				++it;
		}

		static glm::vec3 sizeOffset = glm::vec3( ( float )CHUNK_SIZE, ( float )CHUNK_HEIGHT, ( float )CHUNK_SIZE );
		Aabb             bounds;
		for ( auto & pos : chunksToBuild ) {
			auto chunk = popChunkFromPool();
			chunk->position = pos;
			chunk->worldPosition =
			    glm::i32vec3( ( s32 )getXCoord( pos ) * CHUNK_SIZE, 0, ( s32 )getZCoord( pos ) * CHUNK_SIZE );
			chunk->biome = eBiome::MOUNTAIN;
			bounds.min = chunk->worldPosition;
			bounds.max = glm::vec3( chunk->worldPosition ) + sizeOffset;
			if ( frustrum->IsCubeIn( bounds ) )
				buildingQueueIn.push_back( chunk );
			else
				buildingQueueIn.push_front( chunk );
		}

		queueInMutex.unlock();

		ucMutex.lock();
		updateCondition.notify_all();
		ucMutex.unlock();
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
	glUniformMatrix4fv( modelLoc, 1, GL_FALSE, glm::value_ptr( glm::mat4( 1.0f ) ) );

	static glm::vec3 sizeOffset = glm::vec3( ( float )CHUNK_SIZE, ( float )CHUNK_HEIGHT, ( float )CHUNK_SIZE );
	Aabb             bounds;

	u32 drawCalls = 0;

	for ( auto & it : chunks ) {
		Chunk * chunk = it.second;

		bounds.min = chunk->worldPosition;
		bounds.max = glm::vec3( chunk->worldPosition ) + sizeOffset;
		// Check if any of eight corners of the chunk is in sight
		if ( frustrum->IsCubeIn( bounds ) ) {
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
