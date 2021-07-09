#pragma once

#include "core/math.h"

struct Material
{
	glm::vec3 albedo;
	float ao;
	float metallic;
	float roughness;

	Material()
	{
		albedo = glm::vec3(1.0);
		ao = 1.0f;
		metallic = 0.1f;
		roughness = 1.0f;
	}
};