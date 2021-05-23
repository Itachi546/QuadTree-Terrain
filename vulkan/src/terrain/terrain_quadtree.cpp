#include "terrain_quadtree.h"
#include "debug/debug_draw.h"

QuadTree::QuadTree(uint32_t depth, uint32_t maxSize) : m_depth(depth), m_size(maxSize)
{
}

void QuadTree::insert(glm::vec3 position)
{

	leafNodes.clear();

	glm::ivec2 center = glm::ivec2(m_size / 2);
	glm::ivec2 halfDimForChild = glm::ivec2(m_size) / 4;
	uint32_t id = 1;

	// Create Child
	glm::ivec2 childs[4] = {
		center + glm::ivec2(-halfDimForChild.x, -halfDimForChild.y),
		center + glm::ivec2(halfDimForChild.x, -halfDimForChild.y),
		center + glm::ivec2(-halfDimForChild.x,  halfDimForChild.y),
		center + glm::ivec2(halfDimForChild.x,  halfDimForChild.y),
	};

	for (int i = 0; i < 4; ++i)
		subdivide(childs[i], position, 1, (id << 2) | i);
	}

void QuadTree::debug()
{
	for (int i = 0; i < leafNodes.size(); ++i)
	{
		glm::vec3 min = glm::vec3(leafNodes[i].min.x, 0.0f, leafNodes[i].min.y);
		glm::vec3 max = glm::vec3(leafNodes[i].max.x, 0.0f, leafNodes[i].max.y);

		glm::vec3 color = glm::vec3(1.0f, 0.0f, 0.0f);
		DebugDraw::draw_line(min, glm::vec3(min.x, 0.0f, max.z), color);
		DebugDraw::draw_line(min, glm::vec3(max.x, 0.0f, min.z), color);

		DebugDraw::draw_line(max, glm::vec3(min.x, 0.0f, max.z), color);
		DebugDraw::draw_line(max, glm::vec3(max.x, 0.0f, min.z), color);
	}

}

void QuadTree::subdivide(glm::ivec2 center, glm::vec3 position, uint32_t depth, uint32_t id)
{
	glm::ivec2 halfDim = glm::ivec2(m_size / static_cast<int>(std::pow(2, depth + 1)));
	if (depth >= m_depth)
	{
		// Leaf Node
		leafNodes.push_back({ center - halfDim, center + halfDim, depth, id});
		return;
	}

	float distance = glm::length(glm::vec3(center.x, 0.0f, center.y) - position);
	if (distance > halfDim.x * 4.0f)
	{
		// Leaf Node
		leafNodes.push_back({ center - halfDim, center + halfDim, depth, id});
		return;
	}

	// Create Child
	glm::ivec2 halfDimForChild = halfDim / 2;
	// Create Child
	glm::ivec2 childs[4] = {
		center + glm::ivec2(-halfDimForChild.x, -halfDimForChild.y),
		center + glm::ivec2(halfDimForChild.x, -halfDimForChild.y),
		center + glm::ivec2(-halfDimForChild.x,  halfDimForChild.y),
		center + glm::ivec2(halfDimForChild.x,  halfDimForChild.y),
	};

	for (int i = 0; i < 4; ++i)
		subdivide(childs[i], position, depth + 1, (id << 2) | i);
}
