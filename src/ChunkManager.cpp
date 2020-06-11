#include "Camera.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "ngLib/nglib.h"
#include "packer_resource_list.h"
#include <ChunkManager.hpp>
#include <Guizmo.hpp>
#include <LZ4.h>
#include <algorithm>
#include <chrono>
#include <imgui/imgui.h>
#include <tracy/Tracy.hpp>

#include <Game.h>

MPMCChunkQueue     buildingQueueOut;
MPMCChunkQueue     unloadQueueOut;
MPMCMegaChunkQueue megaChunksToUnload;

std::atomic_bool buildersShouldRun( true );

ChunkCoordinates GetMegaChunkCoordinates( ChunkCoordinates coords ) {
	ChunkCoordinates truncatedCoords = coords;
	truncatedCoords.x = truncatedCoords.x - ( truncatedCoords.x % MEGA_CHUNK_SIZE );
	truncatedCoords.z = truncatedCoords.z - ( truncatedCoords.z % MEGA_CHUNK_SIZE );
	return truncatedCoords;
}

void MegaChunk::AssignCubesTo( Chunk & chunk ) {
	u32 offsetX = chunk.position.x % MEGA_CHUNK_SIZE;
	u32 offsetZ = chunk.position.z % MEGA_CHUNK_SIZE;
	chunk.blocks = &chunks[ offsetX ][ offsetZ ];
}

void MegaChunk::CopyCubesFrom( const Chunk & chunk ) {
	u32 offsetX = chunk.position.x % MEGA_CHUNK_SIZE;
	u32 offsetZ = chunk.position.z % MEGA_CHUNK_SIZE;
	chunk.blocks->CopyCubesTo( chunks[ offsetX ][ offsetZ ] );
}

void MegaChunk::WriteToFile( const char * filePath ) {
	size_t megaChunkDataSize = sizeof( MegaChunk::chunks );
	size_t megaChunkCompressedBoundSize = ( size_t )LZ4_compressBound( ( int )megaChunkDataSize );
	char * compressedData = new char[ megaChunkCompressedBoundSize ];

	ng::File file;
	bool     opened = file.Open( filePath, ng::File::MODE_WRITE | ng::File::MODE_CREATE | ng::File::MODE_TRUNCATE );
	if ( opened ) {
		int compressedBytes = 0;
		{
			ng::ScopedChrono compressionTime( "Compressing one mega chunk" );
			compressedBytes = LZ4_compress_default( ( char * )&chunks, compressedData, ( int )megaChunkDataSize,
			                                        ( int )megaChunkCompressedBoundSize );
		}
		ng::Printf( "Original size: %llukb, compressed size: %llukb, ratio: %f\n", megaChunkDataSize / 1000,
		            compressedBytes / 1000, compressedBytes / ( float )megaChunkDataSize );
		ng_assert( compressedBytes > 0 );
		{
			ng::ScopedChrono disk( "writing one mega chunk to disk" );
			file.Write( compressedData, compressedBytes );
		}
		file.Close();
		ng::Printf( "Saved mega chunk %s\n", filePath );
	}
	delete[] compressedData;
}

std::list< MegaChunk * > megaChunks;
std::list< MegaChunk * > megaChunkPool;

static void MegaChunkRoutine( MegaChunk * mc, const char * saveFolderPath, const HeightMap * hm ) {
	using namespace std::chrono_literals;

	std::string filePath = saveFolderPath;
	filePath += "/mc.";
	filePath += std::to_string( mc->coords.x );
	filePath += ".";
	filePath += std::to_string( mc->coords.z );
	filePath += ".lz4";

	if ( ng::FileExists( filePath.c_str() ) ) {
		ng::Printf( "Loading mega chunk %d %d from file\n", mc->coords.x, mc->coords.z );
		ng::File chunkFile;
		bool     opened = chunkFile.Open( filePath.c_str(), ng::File::MODE_READ );
		if ( opened ) {
			auto   fileSize = chunkFile.GetSize();
			char * data = new char[ fileSize ];
			chunkFile.Read( data, fileSize );

			auto bytesDecompressed =
			    LZ4_decompress_safe( data, ( char * )&mc->chunks, ( int )fileSize, ( int )sizeof( MegaChunk::chunks ) );
			ng_assert( bytesDecompressed == ( int )sizeof( MegaChunk::chunks ) );

			delete[] data;
			chunkFile.Close();
			mc->isGenerated = true;
		}
	}

	if ( mc->isGenerated == false ) {
		ng::Printf( "Creating new mega chunk %d %d\n", mc->coords.x, mc->coords.z );
		ng::ScopedChrono chrono( "new mega chunk generation" );
		for ( u32 x = 0; x < MEGA_CHUNK_SIZE; x++ ) {
			for ( u32 z = 0; z < MEGA_CHUNK_SIZE; z++ ) {
				ChunkCoordinates localCoords = mc->coords;
				localCoords.x += x;
				localCoords.z += z;
				mc->chunks[ x ][ z ].SetupFromHeightmap( *hm, ChunkToWorldPosition( localCoords ) );
			}
		}
		mc->isGenerated = true;
	}

	while ( buildersShouldRun.load() == true ) {
		Chunk * chunk;
		if ( mc->forceSave.load() == true ) {
			mc->WriteToFile( filePath.c_str() );
			mc->forceSave.store( false );
		}

		if ( mc->chunksToGenerate.try_dequeue( chunk ) ) {
			mc->AssignCubesTo( *chunk );
			chunk->CreateGeometry();
			mc->chunksInUse++;
			buildingQueueOut.enqueue( chunk );
		} else if ( mc->chunksToUnload.try_dequeue( chunk ) ) {
			if ( chunk->isDirty ) {
			}
			mc->chunksInUse--;
			unloadQueueOut.enqueue( chunk );

			if ( mc->chunksInUse == 0 ) {
				mc->WriteToFile( filePath.c_str() );
				ng::Printf( "Exiting thread for chunk %d %d\n", mc->coords.x, mc->coords.z );
				megaChunksToUnload.enqueue( mc );
				return;
			}
		} else {
			std::this_thread::sleep_for( 16ms );
		}
	}
}

MegaChunk * FindMegaChunk( ChunkCoordinates coords ) {
	ChunkCoordinates truncatedCoords = GetMegaChunkCoordinates( coords );
	for ( MegaChunk * mc : megaChunks ) {
		if ( mc->coords == truncatedCoords ) {
			return mc;
		}
	}
	return nullptr;
}

MegaChunk * FindOrCreateMegaChunk( ChunkCoordinates coords, const char * saveFolderPath, const HeightMap & hm ) {
	MegaChunk * found = FindMegaChunk( coords );
	if ( found != nullptr ) {
		return found;
	}

	ChunkCoordinates truncatedCoords = GetMegaChunkCoordinates( coords );
	MegaChunk *      newChunk = nullptr;
	if ( megaChunkPool.size() > 0 ) {
		newChunk = megaChunkPool.front();
		megaChunkPool.pop_front();
	} else {
		newChunk = new MegaChunk();
	}
	megaChunks.push_back( newChunk );
	newChunk->coords = truncatedCoords;
	newChunk->thread = new std::thread( MegaChunkRoutine, newChunk, saveFolderPath, &hm );
	return newChunk;
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

std::string GenerateSaveFolderPath( const char * worldName ) {
	std::string saveFilePath = saveFilesFolderPath;
	saveFilePath += "/";
	saveFilePath += worldName;
	return saveFilePath;
}

void ChunkManager::Init( const char * worldName, const glm::vec3 & playerPos ) {
	ZoneScoped;
	buildersShouldRun.store( true );
	shader.CompileFromResource( SHADERS_VOXEL_VERT, SHADERS_VOXEL_FRAG );
	shadowShader.CompileFromResource( SHADERS_VOXEL_SHADOW_VERT, SHADERS_VOXEL_SHADOW_FRAG );
	textureAtlas = loadTextureAtlas( BLOCKS_PIXEL_PERFECT_PNG, 8, 3 );

	strncpy( this->worldName, worldName, WORLD_NAME_MAX_SIZE );
	saveFolderPath = GenerateSaveFolderPath( worldName );

	LoadWorld( saveFolderPath );

	heightMap.Init( worldSeed );

	ChunkCoordinates position = WorldToChunkPosition( playerPos );
	CreateChunksAroundPlayer( position );
}

void ChunkManager::LoadWorld( const std::string & saveFileFolder ) {
	if ( worldSeed == 0 ) {
		ng::File    seedFile;
		std::string seedFilePath = saveFolderPath + "/seed.txt";
		seedFile.Open( seedFilePath.c_str(), ng::File::MODE_READ );
		u64    fileSize = seedFile.GetSize();
		char * buffer = new char[ fileSize + 1 ];
		seedFile.Read( buffer, fileSize );
		buffer[ fileSize ] = 0;
		worldSeed = atoi( buffer );
		seedFile.Close();
		ng::Printf( "Loaded world with seed %d\n", worldSeed );
	}
}

void ChunkManager::CreateNewWorld( const char * worldName ) {
	strncpy( this->worldName, worldName, WORLD_NAME_MAX_SIZE );
	saveFolderPath = GenerateSaveFolderPath( worldName );

	if ( !ng::FileExists( saveFolderPath.c_str() ) ) {
		ng::CreateDirectory( saveFolderPath.c_str() );
	} else {
		ng_assert( false );
	}
	int seed = 0;
	std::srand( ( u32 )std::time( nullptr ) );
	seed = std::rand();
	ng::Printf( "Generated new seed %d for world\n", seed );
	worldSeed = seed;

	ng::File    seedFile;
	std::string seedFilePath = saveFolderPath + "/seed.txt";
	seedFile.Open( seedFilePath.c_str(), ng::File::MODE_CREATE | ng::File::MODE_TRUNCATE | ng::File::MODE_WRITE );
	std::string seedStr = std::to_string( worldSeed );
	seedFile.Write( seedStr.c_str(), seedStr.length() );
	seedFile.Close();
}

void ChunkManager::Shutdown() {
	ZoneScoped;
	buildersShouldRun.store( false );
	for ( MegaChunk * mc : megaChunks ) {
		mc->thread->join();
		delete mc->thread;
		delete mc;
	}
	megaChunks.clear();
	for ( MegaChunk * mc : megaChunkPool ) {
		delete mc;
	}
	megaChunkPool.clear();
	for ( auto & e : chunks ) {
		e.second->DeleteGLBuffers();
		e.second->Destroy();
		delete e.second;
	}
	chunks.clear();
	while ( poolHead != nullptr ) {
		poolHead->Destroy();
		auto newHead = poolHead->poolNextItem;
		delete poolHead;
		poolHead = newHead;
	}
}

inline bool ChunkManager::ChunkIsLoaded( ChunkCoordinates pos ) const { return chunks.find( pos ) != chunks.end(); }

inline bool ChunkManager::ChunkIsLoaded( u16 x, u16 y ) const {
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
	chunk->worldPosition = ChunkToWorldPosition( coord );
	chunk->biome = eBiome::MOUNTAIN;
	chunk->isDirty = false;
	MegaChunk * megaChunk = FindOrCreateMegaChunk( chunk->position, saveFolderPath.c_str(), heightMap );
	return megaChunk->chunksToGenerate.enqueue( chunk );
}

void ChunkManager::Update( const glm::vec3 & playerPos ) {
	ZoneScoped;
	ChunkCoordinates position = WorldToChunkPosition( playerPos );

	FlushLoadingQueue();

	if ( position == lastPosition )
		return;
	lastPosition = position;

	// Remove chunks outside of unload radius
	for ( auto it = std::begin( chunks ); it != std::end( chunks ); ) {
		auto cpos = it->second->position;
		if ( abs( getXCoord( position ) - getXCoord( cpos ) ) > chunkUnloadRadius ||
		     abs( getZCoord( position ) - getZCoord( cpos ) ) > chunkUnloadRadius ) {
			it->second->DeleteGLBuffers();
			Chunk *     chunk = it->second;
			MegaChunk * megaChunk = FindOrCreateMegaChunk( chunk->position, saveFolderPath.c_str(), heightMap );
			megaChunk->chunksToUnload.enqueue( chunk );
			it = chunks.erase( it );
		} else {
			++it;
		}
	}

	Chunk * flushedChunk = nullptr;
	while ( unloadQueueOut.try_dequeue( flushedChunk ) ) {
		pushChunkToPool( flushedChunk );
	}

	MegaChunk * mc = nullptr;
	while ( megaChunksToUnload.try_dequeue( mc ) ) {
		mc->thread->join();
		delete mc->thread;
		megaChunkPool.push_front( mc );
		megaChunks.remove( mc );
	}

	CreateChunksAroundPlayer( position );
}

void ChunkManager::FlushLoadingQueue() {
	Chunk * builtChunk = nullptr;
	while ( buildingQueueOut.try_dequeue( builtChunk ) ) {
		// TODO: We create a lot of chunks multiple, this should not happend as often
		if ( chunks.find( builtChunk->position ) != chunks.end() ) {
			Chunk * oldChunk = chunks[ builtChunk->position ];
			oldChunk->DeleteGLBuffers();
			MegaChunk * megaChunk = FindOrCreateMegaChunk( oldChunk->position, saveFolderPath.c_str(), heightMap );
			megaChunk->chunksToUnload.enqueue( oldChunk );
		}
		chunks[ builtChunk->position ] = builtChunk;
		builtChunk->CreateGLBuffers();
	}
}

void ChunkManager::SaveWorld() {
	for ( auto * mc : megaChunks ) {
		mc->forceSave.store( true );
	}
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

void ChunkManager::Draw( const Frustrum & frustrum, u32 shadowMap ) {
	ZoneScoped;

	shader.Use();
	int shadowMapLocation = glGetUniformLocation( shader.ID, "shadowMap" );
	glUniform1i( shadowMapLocation, 1 );

	glActiveTexture( GL_TEXTURE0 );
	bindTextureAtlas( textureAtlas );
	glActiveTexture( GL_TEXTURE1 );
	glBindTexture( GL_TEXTURE_2D, shadowMap );

	static glm::vec3 sizeOffset = glm::vec3( ( float )CHUNK_SIZE, ( float )CHUNK_HEIGHT, ( float )CHUNK_SIZE );
	Aabb             bounds;

	drawCallsLastFrame = 0;

	for ( auto & it : chunks ) {
		Chunk * chunk = it.second;

		ng_assert( chunk != nullptr );
		bounds.min = chunk->worldPosition;
		bounds.max = chunk->worldPosition + sizeOffset;
		// Check if any of eight corners of the chunk is in sight
		if ( frustrum.IsCubeIn( bounds ) ) {
			shader.SetVector( "chunkWorldPosition", chunk->worldPosition );
			meshDraw( chunk->mesh );
			drawCallsLastFrame++;
		}
	}

	for ( auto & it : chunks ) {
		Chunk * chunk = it.second;
		ng_assert( chunk != nullptr );
		if ( chunk->transparentMesh->verticesCount == 0 ) {
			continue;
		}
		auto worldPosition = ChunkToWorldPosition( chunk->position );
		bounds.min = worldPosition;
		bounds.max = worldPosition + sizeOffset;
		// Check if any of eight corners of the chunk is in sight
		if ( frustrum.IsCubeIn( bounds ) ) {
			shader.SetVector( "chunkWorldPosition", chunk->worldPosition );
			meshDraw( chunk->transparentMesh );
			drawCallsLastFrame++;
		}
	}
}

void ChunkManager::DrawShadows( const Frustrum & frustrum ) {
	ZoneScoped;
	shadowShader.Use();
	static glm::vec3 sizeOffset = glm::vec3( ( float )CHUNK_SIZE, ( float )CHUNK_HEIGHT, ( float )CHUNK_SIZE );
	Aabb             bounds;

	for ( auto & it : chunks ) {
		Chunk * chunk = it.second;

		ng_assert( chunk != nullptr );
		auto worldPosition = ChunkToWorldPosition( chunk->position );
		bounds.min = worldPosition;
		bounds.max = worldPosition + sizeOffset;
		// Check if any of eight corners of the chunk is in sight
		if ( frustrum.IsCubeIn( bounds ) ) {
			shader.SetVector( "chunkWorldPosition", chunk->worldPosition );
			meshDraw( chunk->mesh );
		}
	}

	// for ( auto & it : chunks ) {
	//	Chunk * chunk = it.second;
	//	ng_assert( chunk != nullptr );
	//	if ( chunk->transparentMesh->verticesCount == 0 ) {
	//		continue;
	//	}
	//	auto worldPosition = ChunkToWorldPosition( chunk->position );
	//	bounds.min = worldPosition;
	//	bounds.max = worldPosition + sizeOffset;
	//	// Check if any of eight corners of the chunk is in sight
	//	if ( frustrum.IsCubeIn( bounds ) ) {
	//		auto translationMatrix = glm::translate( glm::mat4( 1.0f ), worldPosition );
	//		glUniformMatrix4fv( modelLoc, 1, GL_FALSE, glm::value_ptr( translationMatrix ) );
	//		meshDraw( chunk->transparentMesh );
	//		drawCallsLastFrame++;
	//	}
	//}
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

	if ( ImGui::Button( "Save the world" ) ) {
		SaveWorld();
	}
	if ( ImGui::TreeNode( "HeightMap" ) ) {
		bool shouldRegenerateWorld = heightMap.DebugDraw();
		if ( shouldRegenerateWorld ) {
			Shutdown();
			Init( worldName, ChunkToWorldPosition( lastPosition ) );
		}
		ImGui::TreePop();
	}
}
