#pragma once

#include <FastNoise/FastNoise.h>

#include <constants.hpp>
#include <cstdlib>
#include <ctime>
#include <stdio.h>

struct HeightMap {
	static constexpr s32 rockLevel = ( CHUNK_HEIGHT / 2 ) + CHUNK_SIZE * 3;
	static constexpr s32 snowLevel = rockLevel + CHUNK_SIZE * 2;
	static constexpr s32 caveLevel = ( CHUNK_HEIGHT / 2 ) - CHUNK_SIZE;
	static constexpr s32 waterLevel = ( CHUNK_HEIGHT / 2 ) + CHUNK_SIZE;

	void Init( int seed );

	FastNoise heightMapNoise;
	float     surfaceFreq;
	float     GetHeightAt( s32 x, s32 y ) const;

	FastNoise elevationNoise;
	float     elevationFreq;
	float     elevationMultiplier;

	FastNoise moistureNoise;
	float     moistureFreq;

	FastNoise offsetNoise;
	float     offsetFreq;

	FastNoise caveNoise;
	float     caveFreq;

	inline float GetElevationAt( s32 x, s32 y ) const {
		return ( elevationNoise.GetNoise( ( float )x, ( float )y ) + 1.0f ) / 2.0f * elevationMultiplier;
	}

	inline float GetOffsetAt( s32 x, s32 y ) const {
		return ( offsetNoise.GetNoise( ( float )x, ( float )y ) + 1.0f ) / 2.0f * CHUNK_SIZE;
	}

	inline float GetMoistureAt( s32 x, s32 y ) const {
		return ( moistureNoise.GetNoise( ( float )x, ( float )y ) + 1.0f ) / 2.0f;
	}
};
