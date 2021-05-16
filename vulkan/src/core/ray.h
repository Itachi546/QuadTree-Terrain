#pragma once

#include "math.h"

struct Ray
{
	glm::vec3 origin;
	glm::vec3 direction;

	bool intersect_sphere(const glm::vec3& position, float radius, float& min_dist) const
	{
		vec3 oc = origin - position;
		float b = glm::dot(oc, direction);
		float c = glm::dot(oc, oc) - radius * radius;
		float h = b * b - c;
		if (h < 0.0) return false;
		h = sqrt(h);
		min_dist = glm::min(-b - h, -b + h);
		return true;
	}

	bool intersect_box(const glm::vec3& min, const glm::vec3& max, float& min_dist) const 
	{   
		// Ax + By + Cz = d
		// dot(p, n) + w = 0
		// p = r0 + t * rd
		// 
		glm::vec3 m = 1.0f / direction;

		float t_min = (min.x - origin.x) * m.x;
		float t_max = (max.x - origin.x) * m.x;
		if (t_min > t_max)
			std::swap(t_min, t_max);

		float t_min_y = (min.y - origin.y) * m.y;
		float t_max_y = (max.y - origin.y) * m.y;

		if (t_min_y > t_max_y)
			std::swap(t_min_y, t_max_y);

		t_min = glm::max(t_min, t_min_y);
		t_max = glm::min(t_max, t_max_y);

		if (t_min > t_max)
			return false;

		float t_min_z = (min.z - origin.z) * m.z;
		float t_max_z = (max.z - origin.z) * m.z;

		if (t_min_z > t_max_z)
			std::swap(t_min_z, t_max_z);

		t_min = glm::max(t_min, t_min_z);
		t_max = glm::min(t_max, t_max_z);

		if (t_min > t_max)
			return false;

		min_dist = t_min;
		return true;
	}
};

class Entity;
struct RayHit
{
	Entity* entity;
	float t;
};