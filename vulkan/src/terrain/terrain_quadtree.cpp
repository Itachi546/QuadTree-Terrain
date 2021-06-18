#include "terrain_quadtree.h"
#include "debug/debug_draw.h"
#include "terrain_chunkmanager.h"
#include "terrain_chunk.h"
#include "scene/camera.h"
#include "core/frustum.h"
#include "renderer/context.h"
#include "renderer/buffer.h"
#include "renderer/device.h"

QuadTree::QuadTree(Context* context, Ref<TerrainStream> stream, uint32_t depth, uint32_t maxSize, int maxHeight) : m_depth(depth), m_size(maxSize), m_maxHeight(maxHeight), m_stream(stream)
{
	// given a depth d, no of node is given by
	// n = (4 ^ (d + 1) / 3)

	int n = static_cast<int>(std::pow(4, depth + 1)) / 3;
	m_nodes.resize(n);
	manager = CreateRef<TerrainChunkManager>(context, 150);
}

void QuadTree::update(Context* context, Ref<Camera> camera)
{
	m_frameIndex++;
	m_visibleList.clear();
	_update(context, camera, glm::ivec2(m_size / 2), 0, 0);
	manager->update(context, m_stream, glm::ivec3(m_size, m_maxHeight, m_size));
}

bool QuadTree::split(const glm::ivec2& position, const glm::ivec2& size, Ref<Camera> camera)
{
	glm::vec3 camPos = camera->get_position();
	glm::ivec2 min = position - size;
	glm::ivec2 max = position + size;
	BoundingBox box = { glm::vec3(min.x, -m_maxHeight, min.y), glm::vec3(max.x, m_maxHeight, max.y) };

	if (camera->get_frustum()->intersect_box(box))
	{
		float distance = glm::length(glm::vec3(position.x, 0.0f, position.y) - camPos);
		if (distance < size.x * 3.0f)
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
	glm::vec3 camPos = camera->get_position();

	if (m_visibleList.size() == 0)
	{
		_get_visible_list(camera, glm::ivec2(m_size / 2), 0, 0, m_visibleList);
		// Sort from front to back

		std::sort(m_visibleList.begin(), m_visibleList.end(), [&](const TerrainChunk* lhs, const TerrainChunk* rhs)
			{
				glm::ivec2 c1 = lhs->get_center();
				glm::ivec2 c2 = rhs->get_center();
				return glm::distance2(glm::vec3(c1.x, 0.0f, c1.y), camPos) < glm::distance2(glm::vec3(c2.x, 0.0f, c2.y), camPos);
			});
	}

	uint32_t indexCount = manager->indexCount;
	context->set_buffer(manager->ib, 0);
	
	for (auto chunk : m_visibleList)
	{
		glm::ivec2 center = chunk->get_center();
		glm::ivec2 size = chunk->get_max() - chunk->get_min();


		float morphFactor = 1.0f;
		/*
		if (chunk->get_lod_level() > 0)
		{
			float transitionRegionWidth = 0.2f * size.x;
			float dist = glm::distance(camPos, glm::vec3(center.x, 0.0f, center.y)) - size.x;
			dist = glm::min(dist, transitionRegionWidth * 2.0f) - transitionRegionWidth;
			dist /= transitionRegionWidth;
			morphFactor = 1.0f - glm::clamp(dist * 0.5f + 0.5f, 0.0f, 1.0f);
		}
		*/
		//context->set_uniform(ShaderStage::Vertex, sizeof(glm::mat4) + sizeof(glm::vec4), sizeof(float), &morphFactor);

		Ref<VertexBufferView> vb = chunk->vb;
		context->set_buffer(vb->buffer, vb->offset);
		context->draw_indexed(indexCount);
	}
}

void QuadTree::destroy()
{
	manager->destroy();
}

// Direction is N(0), S(1), E(2), W(3)
uint32_t QuadTree::find_neighbour(uint32_t currentNode, Direction direction)
{
	//uint32_t parentNode = (currentNode >> 2);
	ASSERT(0);
	return 0u;
}

void QuadTree::_get_visible_list(Ref<Camera> camera, const glm::ivec2& center, uint32_t parent, uint32_t depth, std::vector<TerrainChunk*>& chunks)
{
	glm::ivec2 halfDim = glm::ivec2(m_size / static_cast<int>(std::pow(2, depth + 1)));

	if (depth == m_depth)
	{
		chunks.push_back(m_nodes[parent].chunk);
		m_totalChunkRendered++;
		return;
	}

	uint32_t firstChild = parent * 4 + 1;
	bool childLoaded = true;
	for (int i = 0; i < 4; ++i)
	{
		TerrainChunk* child = m_nodes[firstChild + i].chunk;
		if (child == nullptr || !child->is_loaded())
		{
			childLoaded = false;
			break;
		}
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
			_get_visible_list(camera, childs[i], parent * 4 + 1 + i, depth + 1, chunks);
	}
	else
	{
		TerrainChunk* chunk = m_nodes[parent].chunk;
		if (chunk != nullptr && chunk->is_loaded())
		{
			// Leaf node
			chunks.push_back(chunk);
			m_totalChunkRendered++;
		}
	}
}

