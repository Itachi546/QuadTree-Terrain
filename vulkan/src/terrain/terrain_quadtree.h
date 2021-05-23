#pragma once

#include "core/math.h"
#include <stdint.h>
#include <vector>

struct Node
{
	glm::ivec2 min;
	glm::ivec2 max;
	uint32_t depth;
	uint32_t id;

	glm::ivec3 get_center() const
	{
		return glm::ivec3(min.x + max.x, 0.0f, min.y + max.y) / 2;
	}
};

class QuadTree
{
public:
	QuadTree(uint32_t depth, uint32_t size);

	void insert(glm::vec3 position);
	void debug();
	// min-max : vec4
	std::vector<Node> leafNodes;
private:
	uint32_t m_depth;
	uint32_t m_size;

	void subdivide(glm::ivec2 min, glm::vec3 position, uint32_t depth, uint32_t id);
};