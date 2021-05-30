#include "terrain_chunkmanager.h"
#include "scene/mesh.h"
#include "terrain_quadtree.h"
#include "terrain_chunk.h"
#include "terrain_stream.h"
#include "scene/camera.h"
#include "renderer/context.h"
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

void populate_index_buffer(Context* context, IndexBuffer* ib, uint32_t vertexCount, uint32_t& indexCount)
{
	std::vector<unsigned int> indices;
	for (uint32_t z = 0; z < vertexCount; ++z)
	{
		for (uint32_t x = 0; x < vertexCount; ++x)
		{
			uint32_t i0 = z * (vertexCount + 1) + x;
			uint32_t i1 = i0 + 1;
			uint32_t i2 = i0 + (vertexCount + 1);
			uint32_t i3 = i2 + 1;

			indices.push_back(i2);
			indices.push_back(i1);
			indices.push_back(i0);

			indices.push_back(i2);
			indices.push_back(i3);
			indices.push_back(i1);
		}
	}
	context->copy(ib, indices.data(), 0, static_cast<uint32_t>(indices.size()) * sizeof(uint32_t));
	indexCount = static_cast<uint32_t>(indices.size());
}

TerrainChunkManager::TerrainChunkManager(Context* context, uint32_t poolSize)  : POOL_SIZE(poolSize)
{
	m_chunkPool.resize(POOL_SIZE);

	uint32_t chunkVBSize = (m_vertexCount + 1) * (m_vertexCount + 1) * sizeof(Vertex);
	m_vb = Device::create_vertexbuffer(BufferUsageHint::StaticDraw, chunkVBSize * POOL_SIZE);

	uint32_t chunkIndexSize = m_vertexCount * m_vertexCount * 6 * sizeof(uint32_t);
	ib = Device::create_indexbuffer(BufferUsageHint::StaticDraw, IndexType::UnsignedInt, chunkIndexSize);
	populate_index_buffer(context, ib, m_vertexCount, indexCount);
	
	for (uint32_t i = 0; i < POOL_SIZE; ++i)
	{
		Ref<VertexBufferView> vb = CreateRef<VertexBufferView>();
		vb->buffer = m_vb;
		vb->offset = i * chunkVBSize;
		vb->size = chunkVBSize;
		m_chunkPool[i] = new TerrainChunk(vb);
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
		chunk->build(context, stream, terrainSize, m_vertexCount);
		break;
	}
}

void TerrainChunkManager::destroy()
{
	for (auto& pool : m_chunkPool)
		delete pool;

	Device::destroy_buffer(m_vb);
	Device::destroy_buffer(ib);
	m_chunkPool.clear();
}
