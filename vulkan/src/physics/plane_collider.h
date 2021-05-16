#pragma once

#include "collider.h"
#include "core/math.h"

class PlaneCollider : public Collider
{
public:
	PlaneCollider(const glm::vec3& normal, float distance) : 
		Collider(ColliderType::Plane),
		normal(normal),
		distance(distance)
	{

	}

	glm::vec3 normal;
	float distance;
};
