#include "terrain_chunkmanager.h"
#include "scene/mesh.h"
#include "terrain_quadtree.h"
#include "terrain_chunk.h"
#include "terrain_stream.h"
#include "scene/camera.h"
#include <algorithm>

TerrainChunk* TerrainChunkManager::get_free_chunk()
{
	if (m_availableList.size() > 0)
	{
		uint32_t availableIndex = m_availableList.top();
		m_availableList.pop();
		return m_chunkPool[availableIndex];
	}
	else
	{
		auto found = std::min_element(m_chunkPool.begin(), m_chunkPool.end(), [](const TerrainChunk* lhs, const TerrainChunk* rhs) {
			return lhs->get_last_frame_index() < rhs->get_last_frame_index();
			});
		ASSERT(found != m_chunkPool.end());
		return *found;
	}
}

TerrainChunkManager::TerrainChunkManager(uint32_t poolSize)  : POOL_SIZE(poolSize)
{
	m_chunkPool.resize(POOL_SIZE);
	for (uint32_t i = 0; i < POOL_SIZE; ++i)
	{
		m_chunkPool[i] = new TerrainChunk();
		m_availableList.push(i);
	}
}

void TerrainChunkManager::add_to_cache(TerrainChunk* chunk)
{
	if (!chunk->is_loaded())
		m_chunkToBeLoaded.push(chunk);
}

void TerrainChunkManager::update(Context* context, Ref<TerrainStream> stream, glm::ivec3 terrainSize)
{
	while (m_chunkToBeLoaded.size() > 0)
	{
		TerrainChunk* chunk = m_chunkToBeLoaded.front();
		m_chunkToBeLoaded.pop();
		chunk->build(context, stream, terrainSize);
		break;
	}
}

void TerrainChunkManager::destroy()
{
	for (auto& pool : m_chunkPool)
	{
		pool->destroy();
		delete pool;
	}

	m_chunkPool.clear();
}
