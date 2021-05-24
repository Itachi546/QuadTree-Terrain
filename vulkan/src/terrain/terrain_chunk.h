#pragma once

#include "core/base.h"
#include "core/math.h"

class TerrainStream;
class Mesh;
class Context;

class TerrainChunk
{
public:
	TerrainChunk();

	// reinitialize current chunk
	void initialize(const glm::ivec2& min, const glm::ivec2& max, uint32_t lod_level);

	void build(Context* context, Ref<TerrainStream> stream, const glm::ivec2& terrainSize);
	void render(Context* context);
	bool is_loaded() { return m_loaded; }

	glm::ivec2 get_min() { return m_min; }
	glm::ivec2 get_max() { return m_max; }

	glm::ivec2 get_center() { return (m_min + m_max) / 2; }
	void destroy();
private:
	Mesh* m_mesh;
	bool m_loaded = false;
	glm::ivec2 m_min;
	glm::ivec2 m_max;
	uint32_t m_lodLevel;
};