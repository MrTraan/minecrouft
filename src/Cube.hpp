#pragma once

#include <glm/glm.hpp>
#include <vector>

#include <Mesh.hpp>

// Cube size is 1x1x1
#define CUBE_SIZE 1;

struct Face {
	Vertex vertices[4];
	unsigned int indices[6];
};

class Cube {
   public:
	Cube(glm::vec3 position);
	// Default constructor
	Cube(){};

	void Init(glm::vec3 position);

	Face Front;
	Face Back;
	Face Top;
	Face Bottom;
	Face Left;
	Face Right;

	enum eFaceDirection {
		FRONT = 0,
		RIGHT = 1,
		BACK = 2,
		LEFT = 3,
		TOP = 4,
		BOTTOM = 5,
	};

	Face GetFace(eFaceDirection direction);

   private:
	Vertex vertices[16];
};
