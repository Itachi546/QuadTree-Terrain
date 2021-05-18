#pragma once

#include "collider.h"
#include "core/math.h"

// Center, Radius

class SphereCollider : public Collider
{
public:
	SphereCollider(glm::vec3 center = glm::vec3(0.0f), float radius = 1.0f) :
		center(center),
		Collider(ColliderType::Sphere),
		radius(radius)
	{

	}

	glm::vec3 center;
	float radius;
};
