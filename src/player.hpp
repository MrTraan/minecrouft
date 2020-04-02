#pragma once

#include <Mesh.hpp>
#include <glm/glm.hpp>
#include "Chunk.hpp"

struct IO;
struct Camera;
struct ChunkManager;

struct Player {
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;

	float speed = 6.0f;
	float cubeSelectionRange = 3.0f;

	struct HitInfo {
		glm::vec3  cubeWorldCoord;
		Chunk *    hitChunk;
		eDirection hitDirection;
	};

	void Init();
	void Update( const IO & io, float dt );
	bool TrySelectingBlock( const IO & io, ChunkManager & chunkManager, HitInfo & hitInfo );

	void Draw( const Camera & camera );

	Mesh      damageSprite;
	Shader    damageSpriteShader;
	bool      isHittingCube = false;
	float     hittingSince = 0.0f;
	glm::vec3 hitCubeWorldCoord;

	void DebugDraw();
};