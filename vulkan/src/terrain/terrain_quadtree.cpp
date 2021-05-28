#include "terrain_quadtree.h"
#include "debug/debug_draw.h"
#include "terrain_chunkmanager.h"
#include "terrain_chunk.h"
#include "scene/camera.h"
#include "core/frustum.h"

QuadTree::QuadTree(Ref<TerrainStream> stream, uint32_t depth, uint32_t maxSize, int maxHeight) : m_depth(depth), m_size(maxSize), m_maxHeight(maxHeight), m_stream(stream)
{
	// given a depth d, no of node is given by
	// n = (4 ^ (d + 1) / 3)

	int n = static_cast<int>(std::pow(4, depth + 1)) / 3;
	m_nodes.resize(n);
	manager = CreateRef<TerrainChunkManager>(100);
}

void QuadTree::update(Context* context, Ref<Camera> camera)
{
	m_frameIndex++;
	_update(context, camera, glm::ivec2(m_size / 2), 0, 0);
	manager->update(context, m_stream, glm::ivec3(m_size, m_maxHeight, m_size));
}

bool QuadTree::split(const glm::ivec2& position, const glm::ivec2& size, Ref<Camera> camera)
{
	glm::ivec2 min = position - size;
	glm::ivec2 max = position + size;
	BoundingBox box = { glm::vec3(min.x, -m_maxHeight, min.y), glm::vec3(max.x, m_maxHeight, max.y) };

	if (camera->get_frustum()->intersect_box(box))
	{
		float distance = glm::length(glm::vec3(position.x, 0.0f, position.y) - camera->get_position());
		if (distance < size.x * 2.0f)
			return true;
		return false;
	}
	return false;
}

void QuadTree::assign_chunk(const glm::ivec2& min, const glm::ivec2& max, uint32_t lod, uint32_t id)
{
	TerrainChunk* chunk = m_nodes[id].chunk;
	if (chunk == nullptr)
	{
		chunk = manager->get_free_chunk();
		if (chunk->get_id() != UINT_MAX)
			m_nodes[chunk->get_id()].chunk = nullptr;
		chunk->initialize(min, max, lod, id, m_frameIndex);
		manager->add_to_cache(chunk);
		m_nodes[id].chunk = chunk;
	}
	else
		chunk->set_last_frame_index(m_frameIndex);
}

void QuadTree::_update(Context* context, Ref<Camera> camera, const glm::ivec2& center, uint32_t parent, uint32_t depth)
{

	if (depth >= m_depth + 1)
		return;

	glm::ivec2 halfDim = glm::ivec2(m_size / static_cast<int>(std::pow(2, depth + 1)));

	assign_chunk(center - halfDim, center + halfDim, m_depth - depth, parent);
	if (split(center, halfDim, camera) || depth == 0)
	{
		glm::ivec2 halfDimForChild = halfDim / 2;
		// Create Child
		glm::ivec2 childs[4] =
		{
			center + glm::ivec2(-halfDimForChild.x, -halfDimForChild.y),
			center + glm::ivec2(halfDimForChild.x, -halfDimForChild.y),
			center + glm::ivec2(-halfDimForChild.x,  halfDimForChild.y),
			center + glm::ivec2(halfDimForChild.x,  halfDimForChild.y),
		};

		uint32_t firstChild = parent * 4 + 1;
		for (int i = 0; i < 4; ++i)
			_update(context, camera, childs[i], firstChild + i, depth + 1);
	}
}

void draw_quad(const glm::ivec2& min_size, const glm::ivec2& max_size)
{
	glm::vec3 min = glm::vec3(min_size.x, 0.0f, min_size.y);
	glm::vec3 max = glm::vec3(max_size.x, 0.0f, max_size.y);

	glm::vec3 color = glm::vec3(1.0f, 0.0f, 0.0f);
	DebugDraw::draw_line(min, glm::vec3(min.x, 0.0f, max.z), color);
	DebugDraw::draw_line(min, glm::vec3(max.x, 0.0f, min.z), color);

	DebugDraw::draw_line(max, glm::vec3(min.x, 0.0f, max.z), color);
	DebugDraw::draw_line(max, glm::vec3(max.x, 0.0f, min.z), color);
}


void QuadTree::render(Context* context, Ref<Camera> camera)
{
	m_totalChunkRendered = 0;
	_render(context, camera, glm::ivec2(m_size / 2), 0, 0);
}

void QuadTree::_render(Context* context, Ref<Camera> camera, const glm::ivec2& center, uint32_t parent, uint32_t depth)
{
	glm::ivec2 halfDim = glm::ivec2(m_size / static_cast<int>(std::pow(2, depth + 1)));

	if (depth == m_depth)
	{
		m_nodes[parent].chunk->render(context);
		m_totalChunkRendered++;
		return;
	}

	uint32_t firstChild = parent * 4 + 1;
	bool childLoaded = true;
	for (int i = 0; i < 4; ++i)
	{
		TerrainChunk* child = m_nodes[firstChild + i].chunk;
		if (child == nullptr || !child->is_loaded())
			childLoaded = false;
	}

	if ((split(center, halfDim, camera) && childLoaded) || depth == 0)
	{
		glm::ivec2 halfDimForChild = halfDim / 2;
		// Create Child
		glm::ivec2 childs[4] =
		{
			center + glm::ivec2(-halfDimForChild.x, -halfDimForChild.y),
			center + glm::ivec2(halfDimForChild.x, -halfDimForChild.y),
			center + glm::ivec2(-halfDimForChild.x,  halfDimForChild.y),
			center + glm::ivec2(halfDimForChild.x,  halfDimForChild.y),
		};

		for (int i = 0; i < 4; ++i)
			_render(context, camera, childs[i], parent * 4 + 1 + i, depth + 1);
	}
	else
	{
		TerrainChunk* chunk = m_nodes[parent].chunk;
		if (chunk != nullptr && chunk->is_loaded())
		{
			chunk->render(context);
			m_totalChunkRendered++;
		}
	}
}

