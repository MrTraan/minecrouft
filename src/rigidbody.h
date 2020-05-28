#pragma once

#include "matrix.h"

struct BoxCollider {
	Vec3 center;
	Vec3 size = {1.0f, 1.0f, 1.0f};
};

struct RigidBody {
	BoxCollider collider;
	Vec3        velocity;
	float       mass = 9.8f;
};