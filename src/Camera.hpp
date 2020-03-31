#pragma once

#include <glm/glm.hpp>
#include "Frustrum.hpp"

constexpr float CAM_DEFAULT_SENSITIVITY = 0.4f;
constexpr float CAM_DEFAULT_ZOOM = 45.0f;

struct Player;
struct IO;

struct Camera {
	// Camera Attributes
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 worldUp;

	glm::mat4 projMatrix;
	glm::mat4 viewMatrix;

	// Euler angles
	float yaw = 45.0f;
	float pitch = 0.0f;
	float viewDistance = 300.0f;
	float fov = glm::radians( 80.0f );

	// Camera options
	float sensitivity = CAM_DEFAULT_SENSITIVITY;
	float zoom = CAM_DEFAULT_ZOOM;

	Frustrum frustrum;

	void Init( float aspectRatio, const glm::vec3 & position, const glm::vec3 up );

	// Reads input and update view
	void Update( const IO & io, Player & player, float dt );

	void DebugDraw();

	void UpdateCameraVectors();
	void UpdateProjectionMatrix( float aspectRatio );
};
