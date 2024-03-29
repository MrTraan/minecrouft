#include "glm/gtc/type_ptr.hpp"
#include <Camera.hpp>
#include <ChunkManager.hpp>
#include <Guizmo.hpp>
#include <IO.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <player.hpp>

#include "Game.h"
#include "packer_resource_list.h"
#include <imgui/imgui.h>

void Player::Init() {
	damageSpriteShader.CompileFromResource( SHADERS_DEFAULT_VERT, SHADERS_DEFAULT_FRAG );

	PrepareTexturedUnitCube( &damageSprite, 20 );
	// PrepareTexturedUnitCube( &damageSprite, 7 );
	meshCreateGLBuffers( &damageSprite );
}

void Player::Update( const IO & io, float dt ) {
	ZoneScoped;
	float moveSpeed = speed * dt;

	if ( io.keyboard.IsKeyDown( eKey::KEY_SPACE ) )
		moveSpeed = 20 * dt;

	glm::vec3 translation( 0.0f, 0.0f, 0.0f );
	if ( io.keyboard.IsKeyDown( eKey::KEY_W ) )
		translation += moveSpeed * front;
	if ( io.keyboard.IsKeyDown( eKey::KEY_S ) )
		translation -= moveSpeed * front;
	if ( io.keyboard.IsKeyDown( eKey::KEY_A ) )
		translation -= glm::normalize( glm::cross( front, up ) ) * moveSpeed;
	if ( io.keyboard.IsKeyDown( eKey::KEY_D ) )
		translation += glm::normalize( glm::cross( front, up ) ) * moveSpeed;

	position += translation;

	HitInfo hitInfo;
	if ( TrySelectingBlock( io, theGame->chunkManager, hitInfo ) ) {
		Guizmo::LinesAroundCube( hitInfo.cubeWorldCoord );
		ChunkManager & chunkManager = theGame->chunkManager;
		glm::vec3      chunkPos = ChunkToWorldPosition( hitInfo.hitChunk->position );
		int            cx = hitInfo.cubeWorldCoord.x - chunkPos.x;
		int            cy = hitInfo.cubeWorldCoord.y - chunkPos.y;
		int            cz = hitInfo.cubeWorldCoord.z - chunkPos.z;

		if ( io.mouse.IsButtonDown( Mouse::Button::LEFT ) ) {
			if ( !isHittingCube || hitCubeWorldCoord != hitInfo.cubeWorldCoord ) {
				// Starting to hit a new cube
				hittingSince = 0.0f;
				isHittingCube = true;
				hitCubeWorldCoord = hitInfo.cubeWorldCoord;
			}
			hittingSince += dt;
			eBlockType blockType = hitInfo.hitChunk->GetBlock( cx, cy, cz );
			if ( hittingSince >= BlockGetResistance( blockType ) ||
			     ( destroyCubeInstant && io.mouse.IsButtonPressed( Mouse::Button::LEFT ) ) ) {
				hitInfo.hitChunk->SetBlock( cx, cy, cz, eBlockType::INACTIVE );
				hitInfo.hitChunk->CreateGeometry();
				hitInfo.hitChunk->UpdateGLBuffers();
			} else {
				// Update damage texture
				float         damageRatio = hittingSince / BlockGetResistance( blockType );
				constexpr int numDamageTexture = 10;
				constexpr int damageTextureOffset = 10;
				UpdateTextureIndexOnUnitCube( &damageSprite,
				                              floor( damageRatio * numDamageTexture ) + damageTextureOffset );
				meshUpdateBuffer( &damageSprite );
			}
		} else {
			isHittingCube = false;
			hittingSince = 0.0f;
		}

		if ( io.mouse.IsButtonPressed( Mouse::Button::RIGHT ) ) {
			int offsetX = 0;
			int offsetY = 0;
			int offsetZ = 0;
			switch ( hitInfo.hitDirection ) {
			case eDirection::BOTTOM:
				offsetY = -1;
				break;
			case eDirection::EAST:
				offsetX = 1;
				break;
			case eDirection::NORTH:
				offsetZ = 1;
				break;
			case eDirection::SOUTH:
				offsetZ = -1;
				break;
			case eDirection::TOP:
				offsetY = 1;
				break;
			case eDirection::WEST:
				offsetX = -1;
				break;
			}

			ChunkCoordinates coord = hitInfo.hitChunk->position;
			Chunk *          chunkToUpdate = hitInfo.hitChunk;
			if ( cx + offsetX >= CHUNK_SIZE ) {
				coord.x++;
				chunkToUpdate = ( chunkManager.chunks.at( coord ) );
				cx = 0;
				offsetX = 0;
			}
			if ( cx + offsetX < 0 ) {
				coord.x--;
				chunkToUpdate = ( chunkManager.chunks.at( coord ) );
				cx = CHUNK_SIZE - 1;
				offsetX = 0;
			}
			if ( cz + offsetZ >= CHUNK_SIZE ) {
				coord.z++;
				chunkToUpdate = ( chunkManager.chunks.at( coord ) );
				cz = 0;
				offsetZ = 0;
			}
			if ( cz + offsetZ < 0 ) {
				coord.z--;
				chunkToUpdate = ( chunkManager.chunks.at( coord ) );
				cz = CHUNK_SIZE - 1;
				offsetZ = 0;
			}

			chunkToUpdate->SetBlock( cx + offsetX, cy + offsetY, cz + offsetZ, eBlockType::STONE );
			chunkToUpdate->CreateGeometry();
			chunkToUpdate->UpdateGLBuffers();
		}

	} else {
		isHittingCube = false;
		hittingSince = 0.0f;
	}
}

void Player::FixedUpdate() {
	return;
	// auto       feetPosition = position - glm::vec3( 0.0f, 1.0f, 0.0f );
	// auto       chunkPosition = WorldToChunkPosition( feetPosition );
	// auto const currentChunk = theGame->chunkManager.GetChunkAt( chunkPosition );
	// auto       chunkWorldPos = ChunkToWorldPosition( currentChunk->position );
	// auto       positionInsideChunk = feetPosition - chunkWorldPos;
	// int        cx = positionInsideChunk.x;
	// int        cy = positionInsideChunk.y;
	// int        cz = positionInsideChunk.z;

	// if ( !BlockTypeIsCollidable( currentChunk->cubes[ cx ][ cy ][ cz ] ) ) {
	//	position.y -= rb.mass * FIXED_TIMESTEP;
	//}
	// feetPosition = position - glm::vec3( 0.0f, 1.0f, 0.0f );
	// positionInsideChunk = feetPosition - chunkWorldPos;
	// cx = positionInsideChunk.x;
	// cy = positionInsideChunk.y;
	// cz = positionInsideChunk.z;
	// while ( BlockTypeIsCollidable( currentChunk->cubes[ cx ][ cy ][ cz ] ) ) {
	//	// push outside the cube
	//	float deltaY = feetPosition.y - ( int )feetPosition.y;
	//	float deltaX = feetPosition.x - ( int )feetPosition.x;

	//	if ( deltaY < 0.5f ) {
	//		feetPosition.y -= 1.0f + deltaY;
	//		cy--;
	//	} else {
	//		feetPosition.y += 1.0f - deltaY;
	//		cy++;
	//	}
	//}
}

static int   sign( float x ) { return ( x > 0 ) ? 1 : ( ( x < 0 ) ? -1 : 0 ); }
static float intBound( float s, float ds ) { return ( ds > 0 ? ( ceil( s ) - s ) / ds : ( s - floor( s ) ) / -ds ); }

bool Player::TrySelectingBlock( const IO & io, ChunkManager & chunkManager, HitInfo & hitInfo ) {
	ZoneScoped;
	glm::vec3 direction = front;
	glm::vec3 origin = position;

	float range = cubeSelectionRange;

	float x = floor( origin.x );
	float y = floor( origin.y );
	float z = floor( origin.z );

	int stepX = sign( direction.x );
	int stepY = sign( direction.y );
	int stepZ = sign( direction.z );

	float tMaxX = intBound( origin.x, direction.x );
	float tMaxY = intBound( origin.y, direction.y );
	float tMaxZ = intBound( origin.z, direction.z );

	float deltaX = stepX / direction.x;
	float deltaY = stepY / direction.y;
	float deltaZ = stepZ / direction.z;

	while ( fabs( origin.x - x ) < range && fabs( origin.y - y ) < range && fabs( origin.z - z ) < range ) {
		eDirection direction;
		if ( tMaxX <= tMaxY && tMaxX <= tMaxZ ) {
			direction = stepX < 0 ? eDirection::EAST : eDirection::WEST;
			x += stepX;
			tMaxX += deltaX;
		} else if ( tMaxY <= tMaxX && tMaxY <= tMaxZ ) {
			direction = stepY < 0 ? eDirection::TOP : eDirection::BOTTOM;
			y += stepY;
			tMaxY += deltaY;
		} else {
			direction = stepZ < 0 ? eDirection::NORTH : eDirection::SOUTH;
			z += stepZ;
			tMaxZ += deltaZ;
		}

		if ( y < 0 || y >= CHUNK_HEIGHT || x < 0 || z < 0 ) {
			// TODO: Check x and z out of max size
			return false;
		}

		ChunkCoordinates targetedChunk = WorldToChunkPosition( glm::vec3( x, y, z ) );
		Chunk *          chunk = chunkManager.GetChunkAt( targetedChunk );
		if ( chunk == nullptr ) {
			return false;
		}
		glm::vec3 chunkPos = ChunkToWorldPosition( chunk->position );

		int cx = x - chunkPos.x;
		int cy = y - chunkPos.y;
		int cz = z - chunkPos.z;

		if ( BlockTypeIsCollidable( chunk->GetBlock( cx, cy, cz ) ) ) {
			hitInfo.cubeWorldCoord = glm::vec3( x, y, z );
			hitInfo.hitChunk = chunk;
			hitInfo.hitDirection = direction;
			return true;
		}
	}

	return false;
}

void Player::Draw() {
	if ( isHittingCube ) {
		damageSpriteShader.Use();
		int  modelLoc = glGetUniformLocation( damageSpriteShader.ID, "model" );
		auto translationMatrix = glm::translate( glm::mat4( 1.0f ), hitCubeWorldCoord );
		// auto translationMatrix = glm::mat4( 1.0f );
		// translationMatrix = glm::translate( translationMatrix, glm::vec3( 8208.0f, 167.0f, 8187.0f ) );
		// translationMatrix = glm::scale( translationMatrix, glm::vec3( 1.01f, 1.01f, 1.01f ) );
		// translationMatrix = glm::translate( translationMatrix, glm::vec3( -0.005f, -0.005f, -0.005f ) );
		// translationMatrix = glm::scale( translationMatrix, glm::vec3(1.1f, 1.1f, 1.1f ) );
		// translationMatrix = glm::translate( translationMatrix, glm::vec3( -0.05f, -0.05f, -0.05f));
		glUniformMatrix4fv( modelLoc, 1, GL_FALSE, glm::value_ptr( translationMatrix ) );

		bindTextureAtlas( theGame->chunkManager.textureAtlas );

		glEnable( GL_POLYGON_OFFSET_FILL );
		glPolygonOffset( -1.0f, -1.0f );
		meshDraw( &damageSprite );
		glPolygonOffset( 0.0f, 0.0f );
		glDisable( GL_POLYGON_OFFSET_FILL );
	}
}

void Player::DebugDraw() {
	ImGui::Text( "Hitting since: %f", hittingSince );
	ImGui::Checkbox( "Destroy cube instant", &destroyCubeInstant );
}