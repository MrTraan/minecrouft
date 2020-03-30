#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Aabb {
	glm::vec3 min;
	glm::vec3 max;
};

struct Frustrum {
	void Update(const glm::mat4 & view, const glm::mat4 & projection);
	bool IsCubeIn(const Aabb& aabb) const;
	glm::vec4 planes[6];
};
