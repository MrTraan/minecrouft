#include <HeightMap.hpp>
#include <imgui/imgui.h>
#include <math.h>
#include <tracy/Tracy.hpp>

void HeightMap::Init( int seed ) {
	this->seed = seed;
	elevationNoise.SetSeed( seed );
	elevationNoise.SetNoiseType( FastNoise::Perlin );
	elevationNoise.SetFrequency( elevationFreq );

	heightMapNoise.SetSeed( seed );
	heightMapNoise.SetNoiseType( FastNoise::Perlin );
	heightMapNoise.SetFrequency( surfaceFreq );

	caveNoise.SetSeed( seed );
	caveNoise.SetNoiseType( FastNoise::Perlin );
	caveNoise.SetFrequency( caveFreq );

	moistureNoise.SetSeed( seed );
	moistureNoise.SetNoiseType( FastNoise::Perlin );
	moistureNoise.SetFrequency( moistureFreq );

	offsetNoise.SetSeed( seed );
	offsetNoise.SetNoiseType( FastNoise::Perlin );
	offsetNoise.SetFrequency( offsetFreq );

	treeNoise.SetSeed( seed );
	treeNoise.SetNoiseType( FastNoise::Perlin );
	treeNoise.SetFrequency( treeFreq );
}

float HeightMap::GetHeightAt( s32 x, s32 y ) const {
	float exponant = GetElevationAt( x, y );
	float offset = GetOffsetAt( x, y );

	float height = ( heightMapNoise.GetNoise( x, y ) + 1 ) * CHUNK_SIZE +
	               ( heightMapNoise.GetNoise( x * 2, y * 2 ) + 1 ) * 0.5f * CHUNK_SIZE +
	               ( heightMapNoise.GetNoise( x * 4, y * 4 ) + 1 ) * 0.25f * CHUNK_SIZE;

	return pow( height, exponant ) + offset;
}

bool HeightMap::DebugDraw() {
	ImGui::Text( "Seed: %d\n", seed );
	ImGui::SliderFloat( "Moisture", &moistureFreq, 0.0f, 1.0f );
	ImGui::SliderFloat( "Offset", &offsetFreq, 0.0f, 1.0f );
	ImGui::SliderFloat( "Cave", &caveFreq, 0.0f, 1.0f );
	ImGui::SliderFloat( "Tree", &treeFreq, 0.0f, 1.0f );
	ImGui::SliderFloat( "Elevation", &elevationFreq, 0.0f, 1.0f );
	if ( ImGui::Button( "Reset" ) ) {
		Init( seed );
		return true;
	}
	return false;
}
