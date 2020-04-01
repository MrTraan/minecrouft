#include "ChunkManager.hpp"
#include <Chunk.hpp>
#include <HeightMap.hpp>
#include <math.h>
#include <tracy/Tracy.hpp>

static constexpr s32 rockLevel = ( CHUNK_HEIGHT / 2 ) + CHUNK_SIZE * 3;
static constexpr s32 snowLevel = rockLevel + CHUNK_SIZE * 2;
static constexpr s32 caveLevel = ( CHUNK_HEIGHT / 2 ) - CHUNK_SIZE;
static constexpr s32 waterLevel = ( CHUNK_HEIGHT / 2 ) + CHUNK_SIZE;

HeightMap::HeightMap() {
	std::srand( std::time( nullptr ) );
	int seed = std::rand();

	elevationFreq = 0.003f;
	elevationNoise.SetSeed( seed );
	elevationNoise.SetNoiseType( FastNoise::Perlin );
	elevationNoise.SetFrequency( elevationFreq );

	surfaceFreq = 0.025f;
	heightMapNoise.SetSeed( seed );
	heightMapNoise.SetNoiseType( FastNoise::Perlin );
	heightMapNoise.SetFrequency( surfaceFreq );

	caveFreq = 0.08f;
	caveNoise.SetSeed( seed );
	caveNoise.SetNoiseType( FastNoise::Perlin );
	caveNoise.SetFrequency( caveFreq );

	moistureFreq = 0.08f;
	moistureNoise.SetSeed( seed );
	moistureNoise.SetNoiseType( FastNoise::Perlin );
	moistureNoise.SetFrequency( moistureFreq );

	offsetFreq = 0.015f;
	offsetNoise.SetSeed( seed );
	offsetNoise.SetNoiseType( FastNoise::Perlin );
	offsetNoise.SetFrequency( offsetFreq );

	elevationMultiplier = 1.55f;
}

float HeightMap::GetHeightAt( s32 x, s32 y ) const {
	float exponant = GetElevationAt( x, y );
	float offset = GetOffsetAt( x, y );

	float height = ( heightMapNoise.GetNoise( x, y ) + 1 ) * CHUNK_SIZE +
	               ( heightMapNoise.GetNoise( x * 2, y * 2 ) + 1 ) * 0.5f * CHUNK_SIZE +
	               ( heightMapNoise.GetNoise( x * 4, y * 4 ) + 1 ) * 0.25f * CHUNK_SIZE;

	return pow( height, exponant ) + offset;
}

void HeightMap::SetupChunk( Chunk * chunk ) const {
	ZoneScoped;

	auto chunkPos = ChunkToWorldPosition( chunk->position );

	for ( s32 x = 0; x < CHUNK_SIZE; x++ ) {
		for ( s32 z = 0; z < CHUNK_SIZE; z++ ) {
			// auto type = ( caveNoise.GetNoise( chunkPos.x + x, chunkPos.z + z ) + 1.f ) / 2.f;
			for ( s32 y = 0; y < caveLevel; y++ ) {
				// auto height = ( caveNoise.GetNoise( chunkPos.x + x, chunkPos.z + z, chunkPos.y + y ) );
				// if ( height > 0.0f ) {
				//	if ( type < 0.33f )
				//		chunk->cubes[ x ][ y ][ z ] = eBlockType::SAND;
				//	else if ( type < 0.5f )
				//		chunk->cubes[ x ][ y ][ z ] = eBlockType::DIRT;
				//	else
				//		chunk->cubes[ x ][ y ][ z ] = eBlockType::ROCK;
				//} else
				//	chunk->cubes[ x ][ y ][ z ] = eBlockType::INACTIVE;
				chunk->cubes[ x ][ y ][ z ] = eBlockType::ROCK;
			}

			auto height = GetHeightAt( chunkPos.x + x, chunkPos.z + z ) + CHUNK_HEIGHT / 2;
			auto type = GetMoistureAt( chunkPos.x + x, chunkPos.z + z );
			for ( s32 y = caveLevel; y < CHUNK_HEIGHT; y++ ) {
				if ( y < height ) {
					if ( height < rockLevel ) {
						chunk->cubes[ x ][ y ][ z ] = eBlockType::GRASS;
					} else if ( height < snowLevel ) {
						if ( type < 0.5f )
							chunk->cubes[ x ][ y ][ z ] = eBlockType::GRASS;
						else
							chunk->cubes[ x ][ y ][ z ] = eBlockType::SNOW;
					} else
						chunk->cubes[ x ][ y ][ z ] = eBlockType::SNOW;
				} else if ( y < waterLevel ) {
					chunk->cubes[ x ][ y ][ z ] = eBlockType::WATER;
				} else {
					chunk->cubes[ x ][ y ][ z ] = eBlockType::INACTIVE;
				}
			}
		}
	}
}
