#pragma once

#include "collider.h"
#include "core/math.h"

class SphereCollider : public Collider
{
public:
	SphereCollider(float radius) :
		Collider(ColliderType::Sphere),
		radius(radius)
	{

	}

	float radius;
};
