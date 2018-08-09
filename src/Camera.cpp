#include <imgui/imgui.h>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <limits.h>

#include <Camera.hpp>
#include <Keyboard.hpp>
#include <Mouse.hpp>
#include <constants.hpp>

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch) {
	this->Position = position;
	this->WorldUp = up;
	this->Yaw = yaw;
	this->Pitch = pitch;
	this->updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() {
	return glm::lookAt(this->Position, this->Position + this->Front, this->Up);
}

void Camera::updateCameraVectors() {
	glm::vec3 front;

	front.x = cosf(glm::radians(this->Yaw)) * cosf(glm::radians(this->Pitch));
	front.y = sinf(glm::radians(this->Pitch));
	front.z = sinf(glm::radians(this->Yaw)) * cosf(glm::radians(this->Pitch));
	this->Front = glm::normalize(front);

	this->Right = glm::normalize(glm::cross(this->Front, this->WorldUp));
	this->Up = glm::normalize(glm::cross(this->Right, this->Front));
}

void Camera::Update(float dt) {
	float speed = 6 * dt;

	if (Keyboard::IsKeyDown(eKey::KEY_SPACE))
		speed *= 20.0f;

	if (Keyboard::IsKeyDown(eKey::KEY_W))
		this->Position += speed * this->Front;
	if (Keyboard::IsKeyDown(eKey::KEY_S))
		this->Position -= speed * this->Front;
	if (Keyboard::IsKeyDown(eKey::KEY_A))
		this->Position -=
		    glm::normalize(glm::cross(this->Front, this->Up)) * speed;
	if (Keyboard::IsKeyDown(eKey::KEY_D))
		this->Position +=
		    glm::normalize(glm::cross(this->Front, this->Up)) * speed;

	if (this->Position.x < 0)
		this->Position.x = 0;
	if (this->Position.x > SHRT_MAX * CHUNK_SIZE)
		this->Position.x = SHRT_MAX * CHUNK_SIZE;
	if (this->Position.y < 0)
		this->Position.y = 0;
	if (this->Position.y > CHUNK_HEIGHT + CHUNK_SIZE)
		this->Position.y = CHUNK_HEIGHT + CHUNK_SIZE;
	if (this->Position.z < 0)
		this->Position.z = 0;
	if (this->Position.z > SHRT_MAX * CHUNK_SIZE)
		this->Position.z = SHRT_MAX * CHUNK_SIZE;

	float sensitivity = 0.1f;
	int yDirection = -1;
	this->Yaw += Mouse::Offset.x * sensitivity;
	this->Pitch += Mouse::Offset.y * sensitivity * yDirection;

	if (this->Pitch > 89.0f)
		this->Pitch = 89.0f;
	if (this->Pitch < -89.0f)
		this->Pitch = -89.0f;
	this->updateCameraVectors();

	ImGui::Text("Camera position: %f, %f, %f", Position.x, Position.y,
	            Position.z);
	ImGui::Text("Yaw: %f Pitch: %f", Yaw, Pitch);
}
