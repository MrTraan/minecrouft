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

bool Player::TrySelectingBlock( const IO & io, const ChunkManager & chunkManager, const Camera & camera ) {
	glm::vec3 direction = camera.front;
	glm::vec3 origin = camera.position;

	float range = cubeSelectionRange;

	float oX = origin.x;
	float oY = origin.y;
	float oZ = origin.z;

	float x = floor( oX );
	float y = floor( oY );
	float z = floor( oZ );

	float dX = direction.x;
	float dY = direction.y;
	float dZ = direction.z;

	int stepX = sign( dX );
	int stepY = sign( dY );
	int stepZ = sign( dZ );

	float tMaxX = intBound( oX, dX );
	float tMaxY = intBound( oY, dY );
	float tMaxZ = intBound( oZ, dZ );

	float deltaX = stepX / dX;
	float deltaY = stepY / dY;
	float deltaZ = stepZ / dZ;

	while ( fabs( oX - x ) < range && fabs( oY - y ) < range && fabs( oZ - z ) < range ) {
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

		for ( auto it : chunkManager.chunks ) {
			Chunk &   chunk = *it.second;
			glm::vec3 chunkPos = ChunkToWorldPosition( chunk.position );

			if ( x >= chunkPos.x && x < chunkPos.x + CHUNK_SIZE && y >= 0 && y < CHUNK_HEIGHT && z >= chunkPos.z &&
			     z < chunkPos.z + CHUNK_SIZE ) {

				int cx = x - chunkPos.x;
				int cy = y - chunkPos.y;
				int cz = z - chunkPos.z;

				if ( chunk.cubes[ cx ][ cy ][ cz ] != eBlockType::INACTIVE ) {

					float     o = 0.001f;
					glm::vec3 ov( -o, -o, -o );

					Guizmo::Line( glm::vec3( x, y, z ) + ov, glm::vec3( x + 1 + 2 * o, y, z ) + ov, Guizmo::colWhite );
					Guizmo::Line( glm::vec3( x, y, z ) + ov, glm::vec3( x, y, z + 1 + 2 * o ) + ov, Guizmo::colWhite );
					Guizmo::Line( glm::vec3( x, y, z + 1 + 2 * o ) + ov,
					              glm::vec3( x + 1 + 2 * o, y, z + 1 + 2 * o ) + ov, Guizmo::colWhite );
					Guizmo::Line( glm::vec3( x + 1 + 2 * o, y, z ) + ov,
					              glm::vec3( x + 1 + 2 * o, y, z + 1 + 2 * o ) + ov, Guizmo::colWhite );

					Guizmo::Line( glm::vec3( x, y + 1 + 2 * o, z ) + ov,
					              glm::vec3( x + 1 + 2 * o, y + 1 + 2 * o, z ) + ov, Guizmo::colWhite );
					Guizmo::Line( glm::vec3( x, y + 1 + 2 * o, z ) + ov,
					              glm::vec3( x, y + 1 + 2 * o, z + 1 + 2 * o ) + ov, Guizmo::colWhite );
					Guizmo::Line( glm::vec3( x, y + 1 + 2 * o, z + 1 + 2 * o ) + ov,
					              glm::vec3( x + 1 + 2 * o, y + 1 + 2 * o, z + 1 + 2 * o ) + ov, Guizmo::colWhite );
					Guizmo::Line( glm::vec3( x + 1 + 2 * o, y + 1 + 2 * o, z ) + ov,
					              glm::vec3( x + 1 + 2 * o, y + 1 + 2 * o, z + 1 + 2 * o ) + ov, Guizmo::colWhite );

					Guizmo::Line( glm::vec3( x, y, z ) + ov, glm::vec3( x, y + 1 + 2 * o, z ) + ov, Guizmo::colWhite );
					Guizmo::Line( glm::vec3( x + 1 + 2 * o, y, z ) + ov,
					              glm::vec3( x + 1 + 2 * o, y + 1 + 2 * o, z ) + ov, Guizmo::colWhite );
					Guizmo::Line( glm::vec3( x, y, z + 1 + 2 * o ) + ov,
					              glm::vec3( x, y + 1 + 2 * o, z + 1 + 2 * o ) + ov, Guizmo::colWhite );
					Guizmo::Line( glm::vec3( x + 1 + 2 * o, y, z + 1 + 2 * o ) + ov,
					              glm::vec3( x + 1 + 2 * o, y + 1 + 2 * o, z + 1 + 2 * o ) + ov, Guizmo::colWhite );

					std::string sDirection;
					switch ( direction ) {
					case eDirection::BOTTOM:
						sDirection = "BOTTOM";
						break;
					case eDirection::EAST:
						sDirection = "EAST";
						break;
					case eDirection::NORTH:
						sDirection = "NORTH";
						break;
					case eDirection::SOUTH:
						sDirection = "SOUTH";
						break;
					case eDirection::TOP:
						sDirection = "TOP";
						break;
					case eDirection::WEST:
						sDirection = "WEST";
						break;
					}
					ImGui::Text( "Direction: %s\n", sDirection.c_str() );

					if ( io.mouse.IsButtonPressed( Mouse::Button::LEFT ) ) {
						chunk.cubes[ cx ][ cy ][ cz ] = eBlockType::INACTIVE;
						chunkCreateGeometry( &chunk );
						meshUpdateBuffer( chunk.mesh );
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

						ChunkCoordinates coord = chunk.position;
						Chunk *          chunkToUpdate = &chunk;
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
						ng::Printf( "Creating cubes at %d %d %d for chunk %d %d\n", cx + offsetX, cy + offsetY, cz + offsetZ, chunkToUpdate->position.x, chunkToUpdate->position.z );
						ng::Printf("Before: %d %d\n", chunk.mesh->VerticesCount, chunk.mesh->IndicesCount);
						chunkCreateGeometry( chunkToUpdate );
						meshUpdateBuffer( chunkToUpdate->mesh );
						ng::Printf("After: %d %d\n", chunk.mesh->VerticesCount, chunk.mesh->IndicesCount);
					}

					return true;
				}
			}
		}
	}

	return false;
}
