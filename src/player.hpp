#pragma once

#include "Chunk.hpp"
#include <Mesh.hpp>
#include <glm/glm.hpp>
#include "rigidbody.h"

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
	void FixedUpdate();
	bool TrySelectingBlock( const IO & io, ChunkManager & chunkManager, HitInfo & hitInfo );

	void Draw();

	bool grounded = false;
	RigidBody rb;
	VoxelMesh damageSprite;
	Shader    damageSpriteShader;
	bool      isHittingCube = false;
	float     hittingSince = 0.0f;
	bool      destroyCubeInstant = false;
	glm::vec3 hitCubeWorldCoord;

	void DebugDraw();
};