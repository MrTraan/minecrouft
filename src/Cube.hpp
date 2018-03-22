#pragma once

#include <glm/glm.hpp>
#include <vector>

#include <Mesh.hpp>

// Cube size is 1x1x1
#define CUBE_SIZE 1;

class Cube {
   public:
	Cube(glm::vec3 position);

	void Draw(Shader shader);

   private:
	Mesh mesh;
};
