#pragma once

#include "math.h"

struct Ray
{
	glm::vec3 origin;
	glm::vec3 direction;

	bool intersect_sphere(glm::vec3 position, float radius)
	{
		float a = glm::dot(direction, direction);
		float b = glm::dot(direction, origin);
		float c = glm::dot(origin,    origin) - radius;

		return (b * b - a * c) >= 0;
	}
};