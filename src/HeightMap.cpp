#include <HeightMap.hpp>
#include <math.h>
#include <tracy/Tracy.hpp>

void HeightMap::Init( int seed ) {
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
