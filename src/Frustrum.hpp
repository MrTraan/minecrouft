#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Aabb {
	glm::vec3 min;
	glm::vec3 max;
};

class Frustrum {
   public:
	Frustrum(glm::mat4 projection) : Projection(projection) {}

	glm::mat4 Projection;

	void Update(glm::mat4 view);
	bool IsCubeIn(const Aabb& aabb);

   private:
	glm::vec4 planes[6];
};
