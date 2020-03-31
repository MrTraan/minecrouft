#pragma once

#include <glm/glm.hpp>

struct Camera;

namespace Guizmo {
	constexpr glm::vec3 colWhite(1.0f, 1.0f, 1.0f);

	void Init();
	void NewFrame();
	void Line( glm::vec3 a, glm::vec3 b, glm::vec3 color );
	void Draw( const Camera & camera );
};