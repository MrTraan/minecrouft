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

bool Frustrum::IsCubeIn(const Aabb& aabb) {
	glm::vec4 points[] =
	{
	{ aabb.min.x, aabb.min.y, aabb.min.z, 1.0f },
	{ aabb.max.x, aabb.min.y, aabb.min.z, 1.0f },
	{ aabb.max.x, aabb.max.y, aabb.min.z, 1.0f },
	{ aabb.min.x, aabb.max.y, aabb.min.z, 1.0f },

	{ aabb.min.x, aabb.min.y, aabb.max.z, 1.0f },
	{ aabb.max.x, aabb.min.y, aabb.max.z, 1.0f },
	{ aabb.max.x, aabb.max.y, aabb.max.z, 1.0f },
	{ aabb.min.x, aabb.max.y, aabb.max.z, 1.0f }
	};

	// for each plane...
	for (int i = 0; i < 6; ++i)
	{
		bool inside = false;

		for (int j = 0; j < 8; ++j) {
			if (glm::dot(points[j], planes[i]) > 0) {
				inside = true;
				break;
			}
		}

		if (!inside)
			return false;
	}

	return true;
}
