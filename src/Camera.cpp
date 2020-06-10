#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui/imgui.h>
#include <limits.h>

#include "Guizmo.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "tracy/Tracy.hpp"
#include <Camera.hpp>
#include <IO.hpp>
#include <Player.hpp>
#include <constants.hpp>

void Camera::Init( float aspectRatio, const glm::vec3 & position, const glm::vec3 up ) {
	this->position = position;
	this->worldUp = up;
	this->aspectRatio = aspectRatio;
	UpdateProjectionMatrix( aspectRatio );

	glGenBuffers( 1, &uboMatrices );
	glBindBuffer( GL_UNIFORM_BUFFER, uboMatrices );
	glBufferData( GL_UNIFORM_BUFFER, 4 * sizeof( glm::mat4 ) + sizeof( glm::vec4 ) * 2, nullptr, GL_STATIC_DRAW );
	glBindBuffer( GL_UNIFORM_BUFFER, 0 );

	glBindBufferRange( GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 4 * sizeof( glm::mat4 ) + 2 * sizeof( glm::vec4 ) );

	UpdateCameraVectors();
	lightViewMatrix = glm::mat4( 1.0f );
}

void Camera::UpdateCameraVectors() {
	glm::vec3 front;

	front.x = cosf( glm::radians( this->yaw ) ) * cosf( glm::radians( this->pitch ) );
	front.y = sinf( glm::radians( this->pitch ) );
	front.z = sinf( glm::radians( this->yaw ) ) * cosf( glm::radians( this->pitch ) );
	this->front = glm::normalize( front );

	auto right = glm::normalize( glm::cross( this->front, this->worldUp ) );
	this->up = glm::normalize( glm::cross( right, this->front ) );

	viewMatrix = glm::lookAt( this->position, this->position + this->front, this->up );
}

void Camera::UpdateProjectionMatrix( float aspectRatio ) {
	this->aspectRatio = aspectRatio;
	projMatrix = glm::perspective( fov, aspectRatio, zNear, zFar );
}

static glm::vec4 computeLightSpaceFrustumCorner( const glm::mat4 & lightViewMatrix,
                                                 glm::vec3         startPoint,
                                                 glm::vec3         direction,
                                                 float             width ) {
	glm::vec3 point = startPoint + direction * width;
	glm::vec4 point4f( point.x, point.y, point.z, 1.0f );
	return lightViewMatrix * point4f;
}

void GetBoxMinMax( const glm::vec4 points[ 8 ], glm::vec3 & min, glm::vec3 & max ) {
	min.x = points[ 0 ].x;
	max.x = points[ 0 ].x;
	min.y = points[ 0 ].y;
	max.y = points[ 0 ].y;
	min.z = points[ 0 ].z;
	max.z = points[ 0 ].z;
	for ( u32 i = 1; i < 8; i++ ) {
		const glm::vec4 & point = points[ i ];
		if ( point.x > max.x ) {
			max.x = point.x;
		} else if ( point.x < min.x ) {
			min.x = point.x;
		}
		if ( point.y > max.y ) {
			max.y = point.y;
		} else if ( point.y < min.y ) {
			min.y = point.y;
		}
		if ( point.z > max.z ) {
			max.z = point.z;
		} else if ( point.z < min.z ) {
			min.z = point.z;
		}
	}
}

void Camera::Update( const IO & io, Player & player, float dt ) {
	ZoneScoped;
	constexpr int yDirection = -1;
	if ( !io.mouse.debugMouse ) {
		yaw += io.mouse.offset.x * sensitivity * dt;
		pitch += io.mouse.offset.y * sensitivity * yDirection * dt;
	}

	if ( pitch > 89.0f ) {
		pitch = 89.0f;
	}
	if ( pitch < -89.0f ) {
		pitch = -89.0f;
	}
	UpdateCameraVectors();
	frustrum.Update( viewMatrix, projMatrix );

	// Rotate player
	player.front = front;
	player.up = up;

	// Track player position
	position = player.position;

	lightDirection = glm::normalize( -1.0f * lightPosition );
	{
		float farHalfWidth = shadowZFar * tanf( fov / 2 );
		float nearHalfWidth = shadowZNear * tanf( fov / 2 );
		float farHalfHeight = farHalfWidth / aspectRatio;
		float nearHalfHeight = nearHalfWidth / aspectRatio;

		glm::vec3 toFar = front * shadowZFar;
		glm::vec3 toNear = front * shadowZNear;
		glm::vec3 centerNear = toNear + position;
		glm::vec3 centerFar = toFar + position;

		glm::vec3 rightVector = glm::normalize( glm::cross( front, up ) );

		glm::vec3 farTop = centerFar + up * farHalfHeight;
		glm::vec3 farBottom = centerFar - up * farHalfHeight;
		glm::vec3 nearTop = centerNear + up * nearHalfHeight;
		glm::vec3 nearBottom = centerNear - up * nearHalfHeight;

		glm::vec4 points[ 8 ] = {
		    glm::vec4( farTop + rightVector * farHalfWidth, 1.0f ),
		    glm::vec4( farTop - rightVector * farHalfWidth, 1.0f ),
		    glm::vec4( farBottom + rightVector * farHalfWidth, 1.0f ),
		    glm::vec4( farBottom - rightVector * farHalfWidth, 1.0f ),
		    glm::vec4( nearTop + rightVector * nearHalfWidth, 1.0f ),
		    glm::vec4( nearTop - rightVector * nearHalfWidth, 1.0f ),
		    glm::vec4( nearBottom + rightVector * nearHalfWidth, 1.0f ),
		    glm::vec4( nearBottom - rightVector * nearHalfWidth, 1.0f ),
		};
		glm::vec3 min;
		glm::vec3 max;
		GetBoxMinMax( points, min, max );
		glm::vec3 eye = {
		    ( min.x + max.x ) / 2.0f,
		    ( min.y + max.y ) / 2.0f,
		    ( min.z + max.z ) / 2.0f,
		};

		Guizmo::Line( points[ 0 ], points[ 1 ], Guizmo::colRed );
		Guizmo::Line( points[ 1 ], points[ 3 ], Guizmo::colRed );
		Guizmo::Line( points[ 3 ], points[ 2 ], Guizmo::colRed );
		Guizmo::Line( points[ 2 ], points[ 0 ], Guizmo::colRed );

		Guizmo::Line( points[ 0 ], points[ 4 ], Guizmo::colYellow );
		Guizmo::Line( points[ 1 ], points[ 5 ], Guizmo::colYellow );
		Guizmo::Line( points[ 2 ], points[ 6 ], Guizmo::colYellow );
		Guizmo::Line( points[ 3 ], points[ 7 ], Guizmo::colYellow );

		Guizmo::Line( points[ 4 ], points[ 5 ], Guizmo::colYellow );
		Guizmo::Line( points[ 5 ], points[ 7 ], Guizmo::colYellow );
		Guizmo::Line( points[ 7 ], points[ 6 ], Guizmo::colYellow );
		Guizmo::Line( points[ 6 ], points[ 4 ], Guizmo::colYellow );

		{
			const glm::vec3 center = eye + lightDirection;
			const glm::vec3 right = glm::normalize( glm::cross( lightDirection, worldUp ) );
			const glm::vec3 up = glm::normalize( glm::cross( right, lightDirection ) );
			lightViewMatrix = glm::lookAt( eye, center, up );
		}

		for ( u32 i = 0; i < 8; i++ ) {
			points[ i ] = lightViewMatrix * points[ i ];
		}
		GetBoxMinMax( points, min, max );
		min.z += shadowOffset;
		lightProjectionMatrix = glm::ortho( min.x, max.x, min.y, max.y, min.z, max.z );
	}

	glm::mat4 viewProj = projMatrix * viewMatrix;
	glm::mat4 lightViewProj = lightProjectionMatrix * lightViewMatrix;
	glm::mat4 matrices[ 4 ] = {
	    projMatrix,
	    viewMatrix,
	    viewProj,
	    lightViewProj,
	};
	glm::vec4 vectors[ 2 ] = {
	    glm::vec4( lightPosition, 1.0f ),
	    glm::vec4( lightDirection, 1.0f ),
	};
	glBindBuffer( GL_UNIFORM_BUFFER, uboMatrices );
	glBufferSubData( GL_UNIFORM_BUFFER, 0, sizeof( glm::mat4 ) * 4, matrices );
	glBufferSubData( GL_UNIFORM_BUFFER, sizeof( glm::mat4 ) * 4, sizeof( glm::vec4 ) * 2, vectors );
	glBindBuffer( GL_UNIFORM_BUFFER, 0 );
}

void Camera::DebugDraw() {
	ImGui::PushID( "Camera" );
	ImGui::Text( "Camera position: %f, %f, %f", position.x, position.y, position.z );
	ImGui::Text( "Yaw: %f Pitch: %f", yaw, pitch );
	ImGui::Text( "Aspect ratio: %f", aspectRatio );
	ImGui::DragFloat( "Sensitivity", &sensitivity, 0.1f, 0.0f, 100.0f );
	if ( ImGui::DragFloat( "Z near", &zNear ) ) {
		UpdateProjectionMatrix( aspectRatio );
	}
	if ( ImGui::DragFloat( "Z far", &zFar ) ) {
		UpdateProjectionMatrix( aspectRatio );
	}
	float fovDeg = glm::degrees( fov );
	if ( ImGui::DragFloat( "Fov", &fovDeg ) ) {
		fov = glm::radians( fovDeg );
		Init( aspectRatio, position, worldUp );
	}
	ImGui::DragFloat( "Shadow zNear", &shadowZNear );
	ImGui::DragFloat( "Shadow zFar", &shadowZFar );
	ImGui::DragFloat( "Shadow offset", &shadowOffset );
	ImGui::DragFloat3( "Light position", &lightPosition.x );
	ImGui::DragFloat3( "Light direction", &lightDirection.x );
	ImGui::PopID();
}
