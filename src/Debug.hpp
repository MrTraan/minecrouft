#pragma once

#include <stdio.h>
#include <glm/glm.hpp>

class Debug {
   public:
	static void PrintVec3(glm::vec3 v) {
		printf("x: %f y: %f z: %f\n", v.x, v.y, v.z);
	}
};
