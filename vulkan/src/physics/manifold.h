#pragma once

#include "core/math.h"
#include "core/base.h"
#include "rigidbody.h"

struct ContactPoint
{
	// Pointing from object A to object B
	glm::vec3 normal;
	float penetration;
	glm::vec3 p;
};

class Manifold
{
public:
	Manifold(Ref<Rigidbody> a, Ref<Rigidbody> b) : a(a), b(b){}

	void swap_object()
	{
		std::swap(a, b);
	}

	void add_contact(ContactPoint contactPoint) { contacts.push_back(contactPoint); }

	Ref<Rigidbody> a;
	Ref<Rigidbody> b;
	std::vector<ContactPoint> contacts;
};