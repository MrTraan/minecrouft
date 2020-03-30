#pragma once

#include <glm/glm.hpp>

struct IO;

struct Player {
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;

	float speed = 6.0f;

	void Update( const IO & io, float dt );

	void DebugDraw() {}
};