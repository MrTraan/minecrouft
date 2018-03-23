#pragma once

#include <stdio.h>
#include <glm/glm.hpp>

#include <Cube.hpp>
#include <Mesh.hpp>

class Debug {
   public:
	static void PrintVec3(glm::vec3 v) {
		printf("x: %f y: %f z: %f\n", v.x, v.y, v.z);
	}

	static void PrintFace(Face f) {
		printf("Face vertices: \n");
		for (int i = 0; i < 4; i++) {
			printf("%d: ", i);
			PrintVec3(f.vertices[i].Position);
		}

		printf("Face positions: \n");
		for (int i = 0; i < 6; i++)
			printf("%d ", f.indices[i]);
		printf("\n");
	}
};
