#include "terrain_chunkmanager.h"
#include "scene/mesh.h"
#include "terrain_quadtree.h"
#include "terrain_chunk.h"
#include "terrain_stream.h"
#include "scene/camera.h"
#include <set>
#include <algorithm>

TerrainChunk* TerrainChunkManager::get_free_chunk(Ref<Camera> camera)
{
	if (m_availableList.size() > 0)
	{
		TerrainChunk* chunk = m_availableList.top();
		m_availableList.pop();
		return chunk;
	}
	else
	{
		auto found = std::min_element(m_chunkCache.begin(), m_chunkCache.end(), [](const std::pair<uint32_t, ChunkData>& lhs, const std::pair<uint32_t, ChunkData>& rhs) {
			return lhs.second.lastFrameIndex < rhs.second.lastFrameIndex;
			});
		ASSERT(found != m_chunkCache.end());
		TerrainChunk* chunk = found->second.chunk;
		m_chunkCache.erase(found);
		return chunk;
	}
}

void TerrainChunkManager::init(Ref<TerrainStream> stream, glm::ivec2 terrainSize)
{
	m_terrainSize = terrainSize;
	m_stream = stream;
	m_chunkPool.resize(POOL_SIZE);

	for (int i = 0; i < POOL_SIZE; ++i)
	{
		m_chunkPool[i] = new TerrainChunk();
		m_availableList.push(m_chunkPool[i]);
	}
}

void TerrainChunkManager::update(Context* context, Ref<Camera> camera, std::vector<Node>& visibleChunks, uint32_t maxLod)
{
	glm::vec3 camPos = camera->get_position();
	std::sort(visibleChunks.begin(), visibleChunks.end(), [&](const Node& lhs, const Node& rhs) {
		float d1 = glm::length2(camPos - glm::vec3(lhs.get_center()));
		float d2 = glm::length2(camPos - glm::vec3(rhs.get_center()));
		return d1 < d2;
		});
	frameIndex++;

	m_visibleList.clear();
	for (const auto& node : visibleChunks)
	{
		glm::ivec2 center = (node.min + node.max) / 2;
		uint32_t id = node.id;
		m_visibleList.push_back(id);

		auto found = m_chunkCache.find(id);
		if (found == m_chunkCache.end())
		{
			TerrainChunk* chunk = get_free_chunk(camera); 
			ASSERT(chunk != nullptr);
			chunk->initialize(node.min, node.max, maxLod - node.depth);
			m_chunkCache[id] = ChunkData{ frameIndex, chunk };
		}
	}


	for (auto& chunk_key_val : m_chunkCache)
	{
		TerrainChunk* chunk = chunk_key_val.second.chunk;
		if (!chunk->is_loaded())
		{
			chunk->build(context, m_stream, m_terrainSize);
			break;
		}
	}

}

void TerrainChunkManager::render(Context* context)
{

	std::set<TerrainChunk*> renderList;

	for (auto& id : m_visibleList)
	{
		// Get parent id of current chunk
		// This is used to generate all the sibling
		auto parent_id = (id >> 2);
		bool sib_loaded = true;

		for (int i = 0; i < 4; ++i)
		{
			uint32_t sib_id = (parent_id << 2) | i;
			auto sib = m_chunkCache.find(sib_id);

			// Check if any of the child of chunk sibling exists
			// if sibling is not loaded or any of child of sibling is not present
			// we don't render it
			bool sib_child_exists = false;
			for (int j = 0; j < 4; ++j)
			{
				auto sib_child = m_chunkCache.find((sib_id << 2) | j);
				if (sib_child != m_chunkCache.end())
				{
					sib_child_exists = true;
					break;
				}
			}

			if (sib == m_chunkCache.end())
			{
				if (!sib_child_exists)
				{
					sib_loaded = false;
					break;
				}
			}
			else
			{
				if (!sib->second.chunk->is_loaded())
				{
					sib_loaded = false;
					break;
				}
			}
		}

		TerrainChunk* chunk = nullptr;
		if (sib_loaded)
		{
			auto found = m_chunkCache.find(id);
			if (found != m_chunkCache.end())
			{
				found->second.lastFrameIndex = frameIndex;
				chunk = found->second.chunk;
			}
		}
		else
		{
			auto found = m_chunkCache.find(parent_id);
			if (found != m_chunkCache.end())
			{
				found->second.lastFrameIndex = frameIndex;
				chunk = found->second.chunk;
			}
		}

		if (chunk != nullptr && chunk->is_loaded())
			renderList.insert(chunk);
	}

	for (TerrainChunk* chunk : renderList)
	{
		chunk->render(context);
	}
}

void TerrainChunkManager::destroy()
{
	for (auto& pool : m_chunkPool)
	{
		pool->destroy();
		delete pool;
	}

	if (m_stream)
		m_stream->destroy();

	m_chunkPool.clear();
}
