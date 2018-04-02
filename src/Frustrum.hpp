#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Frustrum {
   public:
	Frustrum(glm::mat4 projection) : Projection(projection) {}

	glm::mat4 Projection;

	void Update(glm::mat4 view);
	bool IsPointIn(glm::vec3 point, int renderSize);

   private:
	float planes[6][4];
	void setPlanes();
};
