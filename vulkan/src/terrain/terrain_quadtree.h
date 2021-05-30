#pragma once

#include "core/base.h"
#include "core/math.h"
#include <stdint.h>
#include <vector>

class TerrainChunk;
class TerrainChunkManager;
class Context;
class Camera;
class TerrainStream;

struct Node
{
	TerrainChunk* chunk;
};

class QuadTree
{
public:
	QuadTree(Context* context, Ref<TerrainStream> stream, uint32_t depth, uint32_t size, int m_maxHeight);

	void update(Context* context, Ref<Camera> camera);
	void render(Context* context, Ref<Camera> camera);
	void destroy();
private:
	std::vector<Node> m_nodes;
	uint32_t m_depth;
	uint32_t m_size;
	int m_maxHeight;
	uint64_t m_frameIndex = 0;
	Ref<TerrainChunkManager> manager;
	Ref<TerrainStream> m_stream;

	void _update(Context* context, Ref<Camera> camera, const glm::ivec2& center, uint32_t parent, uint32_t depth);
	void _get_visible_list(Ref<Camera> camera, const glm::ivec2& center, uint32_t parent, uint32_t depth, std::vector<TerrainChunk*>& chunks);
	void assign_chunk(const glm::ivec2& min, const glm::ivec2& max, uint32_t lod, uint32_t id);

	bool split(const glm::ivec2& center, const glm::ivec2& size, Ref<Camera> camera);

	uint32_t m_totalChunkRendered = 0;
};