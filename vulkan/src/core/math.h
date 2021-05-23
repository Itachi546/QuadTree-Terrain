#pragma once

#define GLM_FORCE_XYZW_ONLY
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
using namespace glm;

#define PI       3.14159265358979323846   // pi
#define PI_2     1.57079632679489661923   // pi/2
#define PI_4     0.785398163397448309616  // pi/4
#define INV_PI   0.318309886183790671538  // 1/pi

struct BoundingBox
{
	glm::vec3 min;
	glm::vec3 max;

	BoundingBox(const BoundingBox& box) : min(box.min), max(box.max){}
	BoundingBox(){}
	BoundingBox(const glm::vec3& min, const glm::vec3& max) : min(min), max(max) {}

	BoundingBox operator=(const BoundingBox& box)
	{
		return BoundingBox{ box.min, box.max };
	}

	BoundingBox transform(glm::mat4 transform)
	{
		BoundingBox box;
		box.min = transform * glm::vec4(min, 1.0f);
		box.max = transform * glm::vec4(max, 1.0f);
		return box;
	}
};
