#pragma once

#include <glm/glm.hpp>

constexpr float CAM_YAW = 45.0f;
constexpr float CAM_PITCH = 0.0f;
constexpr float CAM_SPEED = 2.5f;
constexpr float CAM_SENSITIVITY = 0.1f;
constexpr float CAM_ZOOM = 45.0f;

class Camera {
   public:
	// Camera Attributes
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;

	// Euler angles
	float Yaw;
	float Pitch;

	// Camera options
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;

	// Constructors
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
	       glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
	       float yaw = CAM_YAW,
	       float pitch = CAM_PITCH);

	// Returns the view matrix calculated using Euler Angles and the LookAt
	// Matrix
	glm::mat4 GetViewMatrix();

	// Reads input and update view
	void Update(float dt);

   private:
	void updateCameraVectors();
};
