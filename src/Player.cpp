#include <Camera.hpp>
#include <ChunkManager.hpp>
#include <Guizmo.hpp>
#include <IO.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <player.hpp>

#include <imgui/imgui.h>

void Player::Update( const IO & io, float dt ) {
	float moveSpeed = speed * dt;

	if ( io.keyboard.IsKeyDown( eKey::KEY_SPACE ) )
		moveSpeed = 20 * dt;

	if ( io.keyboard.IsKeyDown( eKey::KEY_W ) )
		position += moveSpeed * front;
	if ( io.keyboard.IsKeyDown( eKey::KEY_S ) )
		position -= moveSpeed * front;
	if ( io.keyboard.IsKeyDown( eKey::KEY_A ) )
		position -= glm::normalize( glm::cross( front, up ) ) * moveSpeed;
	if ( io.keyboard.IsKeyDown( eKey::KEY_D ) )
		position += glm::normalize( glm::cross( front, up ) ) * moveSpeed;
}

static int   sign( float x ) { return ( x > 0 ) ? 1 : ( ( x < 0 ) ? -1 : 0 ); }
static float intBound( float s, float ds ) { return ( ds > 0 ? ( ceil( s ) - s ) / ds : ( s - floor( s ) ) / -ds ); }

bool Player::TrySelectingBlock( const IO & io, ChunkManager & chunkManager, const Camera & camera ) {
	ZoneScoped;
	glm::vec3 direction = camera.front;
	glm::vec3 origin = camera.position;

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

	float deltaX = stepX /  direction.x;
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

		if ( chunk->cubes[ cx ][ cy ][ cz ] != eBlockType::INACTIVE ) {
			Guizmo::LinesAroundCube( glm::vec3( x, y, z ) );

			if ( io.mouse.IsButtonPressed( Mouse::Button::LEFT ) ) {
				chunk->cubes[ cx ][ cy ][ cz ] = eBlockType::INACTIVE;
				chunkCreateGeometry( chunk );
				chunk->UpdateGLBuffers();
			}

			if ( io.mouse.IsButtonPressed( Mouse::Button::RIGHT ) ) {
				int offsetX = 0;
				int offsetY = 0;
				int offsetZ = 0;
				switch ( direction ) {
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

				ChunkCoordinates coord = chunk->position;
				Chunk *          chunkToUpdate = chunk;
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

				chunkToUpdate->cubes[ cx + offsetX ][ cy + offsetY ][ cz + offsetZ ] = eBlockType::ROCK;
				chunkCreateGeometry( chunkToUpdate );
				chunkToUpdate->UpdateGLBuffers();
			}

			return true;
		}
	}

	return false;
}
