#pragma once

#include "core/base.h"
#include "core/math.h"
#include <unordered_map>
#include <vector>
#include <stack>
#include <queue>

class TerrainChunk;
class Context;
class TerrainStream;

class VertexBuffer;
class IndexBuffer;

class TerrainChunkManager
{
public:
	TerrainChunkManager(Context* context, uint32_t poolSize);
	TerrainChunk* get_free_chunk();
	void add_to_cache(TerrainChunk* chunk);

	void update(Context* context, Ref<TerrainStream> stream, glm::ivec3 terrainSize);
	void destroy();

	IndexBuffer* ib;
	uint32_t indexCount = 0;
private:
	uint32_t POOL_SIZE = 100;
	uint32_t m_vertexCount = 128;
	// Chunk Cache
	std::vector<TerrainChunk*> m_chunkPool;
	std::stack<uint32_t> m_availableList;
	std::queue<TerrainChunk*> m_chunkToBeLoaded;

	VertexBuffer* m_vb;
};