#pragma once

#include "core/math.h"

class Plane
{

public:
	Plane() : normal(0.0f), distance(FLT_MAX){
	}

	Plane(glm::vec3 n, float d) : normal(n), distance(d){}

	// Anticlockwise 
	Plane(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2)
	{
		set(p0, p1, p2);
	}

	float get_distance_to(const glm::vec3& p)
	{
		return glm::dot(p, normal) + distance;
	}

	void set(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2)
	{
		normal = glm::normalize(glm::cross(p1 - p0, p2 - p0));
		distance = -glm::dot(normal, p0);
	}

	glm::vec3 normal;
	float distance;

};