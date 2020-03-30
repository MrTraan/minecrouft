#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui/imgui.h>
#include <limits.h>

#include "glm/ext/matrix_clip_space.hpp"
#include <Camera.hpp>
#include <IO.hpp>
#include <Player.hpp>
#include <constants.hpp>

void Camera::Init( float aspectRatio, const glm::vec3 & position, const glm::vec3 up ) {
	this->position = position;
	this->worldUp = up;
	projMatrix = glm::perspective( fov, aspectRatio, 0.1f, viewDistance );
	updateCameraVectors();
}

void Camera::updateCameraVectors() {
	glm::vec3 front;

	front.x = cosf( glm::radians( this->yaw ) ) * cosf( glm::radians( this->pitch ) );
	front.y = sinf( glm::radians( this->pitch ) );
	front.z = sinf( glm::radians( this->yaw ) ) * cosf( glm::radians( this->pitch ) );
	this->front = glm::normalize( front );

	auto right = glm::normalize( glm::cross( this->front, this->worldUp ) );
	this->up = glm::normalize( glm::cross( right, this->front ) );

	viewMatrix = glm::lookAt( this->position, this->position + this->front, this->up );
}

void Camera::Update( const IO & io, Player & player, float dt ) {
	constexpr int yDirection = -1;
	yaw += io.mouse.offset.x * sensitivity;
	pitch += io.mouse.offset.y * sensitivity * yDirection;

	if ( pitch > 89.0f ) {
		pitch = 89.0f;
	}
	if ( pitch < -89.0f ) {
		pitch = -89.0f;
	}
	updateCameraVectors();
	frustrum.Update( viewMatrix, projMatrix );

	// Rotate player
	player.front = front;
	player.up = up;

	// Track player position
	position = player.position;
}

void Camera::DebugDraw() {
	ImGui::PushID( "Camera" );
	ImGui::Text( "Camera position: %f, %f, %f", position.x, position.y, position.z );
	ImGui::Text( "Yaw: %f Pitch: %f", yaw, pitch );
	ImGui::DragFloat( "Sensitivity", &sensitivity, 0.1f, 0.0f, 5.0f );
	ImGui::PopID();
}
