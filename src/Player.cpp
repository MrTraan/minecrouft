#include <player.hpp>
#include <IO.hpp>
#include <glm/gtc/matrix_transform.hpp>

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
