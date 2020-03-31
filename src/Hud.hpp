#pragma once

struct Hud {
	float reticuleLines[3 * 4] = {
		0.0f, 0.2f, 0.0f,
		0.0f, -0.2f, 0.0f,
		0.2f, 0.0f, 0.0f,
		-0.2f, 0.0, 0.0f
	};

	void Draw();
};