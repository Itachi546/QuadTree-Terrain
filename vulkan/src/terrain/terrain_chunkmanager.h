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

class TerrainChunkManager
{
public:
	TerrainChunkManager(uint32_t poolSize);
	TerrainChunk* get_free_chunk();
	void add_to_cache(TerrainChunk* chunk);

	void update(Context* context, Ref<TerrainStream> stream, glm::ivec3 terrainSize);
	void destroy();
private:
	uint32_t POOL_SIZE = 100;
	// Chunk Cache
	std::vector<TerrainChunk*> m_chunkPool;
	std::stack<uint32_t> m_availableList;

	std::queue<TerrainChunk*> m_chunkToBeLoaded;
};