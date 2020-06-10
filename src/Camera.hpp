#pragma once

#include "Frustrum.hpp"
#include "ngLib/types.h"
#include <glm/glm.hpp>

constexpr float CAM_DEFAULT_SENSITIVITY = 25.0f;
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

	glm::vec3 lightPosition = { 10, 20, 10 };
	glm::vec3 lightDirection;
	glm::mat4 lightViewMatrix;
	glm::mat4 lightProjectionMatrix;

	u32 uboMatrices;

	// Euler angles
	float yaw = 45.0f;
	float pitch = 0.0f;
	float zNear = 0.1f;
	float zFar = 300.0f;
	float shadowZNear = 0.1f;
	float shadowZFar = 100.0f;
	float shadowOffset = 0.0f;
	float fov = glm::radians( 90.0f );
	float aspectRatio = 0.0f;

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
