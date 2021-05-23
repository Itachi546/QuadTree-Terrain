#pragma once

#include "core/base.h"
#include "core/math.h"
#include <unordered_map>
#include <vector>
#include <stack>

class Mesh;
class TerrainStream;
class TerrainChunk;
struct Node;
class Context;
class Camera;

class TerrainChunkManager
{
public:
	void init(Ref<TerrainStream> stream, glm::ivec2 terrainSize);
	void update(Context* context, Ref<Camera> camera, std::vector<Node>& visibleChunks, uint32_t maxLod);
	void render(Context* context);
	void destroy();

private:
	struct ChunkData
	{
		uint64_t lastFrameIndex = 0;
		TerrainChunk* chunk;
	};
	static const uint32_t POOL_SIZE = 200;
	glm::ivec2 m_terrainSize;
	// Chunk Cache
	std::unordered_map<uint32_t, ChunkData> m_chunkCache;
	std::vector<TerrainChunk*> m_chunkPool;
	std::stack<TerrainChunk*> m_availableList;

	std::vector<uint32_t> m_visibleList;

	Ref<TerrainStream> m_stream;
	TerrainChunk* get_free_chunk(Ref<Camera> camera);

	uint64_t frameIndex = 0;
};