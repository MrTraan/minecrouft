#include <Chunk.hpp>
#include <Frustrum.hpp>

void Frustrum::Update(glm::mat4 view) {
	glm::mat4 matrix = Projection * view;

	planes[0][0] = matrix[0][3] + matrix[0][0];
	planes[0][1] = matrix[1][3] + matrix[1][0];
	planes[0][2] = matrix[2][3] + matrix[2][0];
	planes[0][3] = matrix[3][3] + matrix[3][0];

	planes[1][0] = matrix[0][3] - matrix[0][0];
	planes[1][1] = matrix[1][3] - matrix[1][0];
	planes[1][2] = matrix[2][3] - matrix[2][0];
	planes[1][3] = matrix[3][3] - matrix[3][0];

	planes[2][0] = matrix[0][3] - matrix[0][1];
	planes[2][1] = matrix[1][3] - matrix[1][1];
	planes[2][2] = matrix[2][3] - matrix[2][1];
	planes[2][3] = matrix[3][3] - matrix[3][1];

	planes[3][0] = matrix[0][3] + matrix[0][1];
	planes[3][1] = matrix[1][3] + matrix[1][1];
	planes[3][2] = matrix[2][3] + matrix[2][1];
	planes[3][3] = matrix[3][3] + matrix[3][1];

	planes[4][0] = matrix[0][3] + matrix[0][2];
	planes[4][1] = matrix[1][3] + matrix[1][2];
	planes[4][2] = matrix[2][3] + matrix[2][2];
	planes[4][3] = matrix[3][3] + matrix[3][2];

	planes[5][0] = matrix[0][3] - matrix[0][2];
	planes[5][1] = matrix[1][3] - matrix[1][2];
	planes[5][2] = matrix[2][3] - matrix[2][2];
	planes[5][3] = matrix[3][3] - matrix[3][2];
}

bool Frustrum::IsPointIn(glm::vec3 point) {
	for (unsigned int i = 0; i < 6; i++) {
		if (planes[i][0] * point.x + (CHUNK_SIZE / 2) + planes[i][1] * point.y +
		        planes[i][2] * point.z + planes[i][3] <=
		    -CHUNK_SIZE)
			return false;
	}
	return true;
}
