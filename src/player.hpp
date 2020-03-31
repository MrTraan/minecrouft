#pragma once

#include <glm/glm.hpp>

struct IO;
struct Camera;
struct ChunkManager;

struct Player {
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;

	float speed = 6.0f;
	float cubeSelectionRange = 3.0f;

	void Update( const IO & io, float dt );
	bool TrySelectingBlock( const IO & io, const ChunkManager & chunkManager, const Camera & camera );

	void DebugDraw() {}
};