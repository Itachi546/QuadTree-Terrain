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
	void initialize(const glm::ivec2& min, const glm::ivec2& max, uint32_t lod_level, uint32_t id, uint64_t lastFrameIndex);
	void build(Context* context, Ref<TerrainStream> stream, const glm::ivec3& terrainSize);
	void render(Context* context);
	bool is_loaded() const { return m_loaded; }

	uint32_t get_id() const { return m_id; }
	uint64_t get_last_frame_index() const { return m_lastFrameIndex; }
	void set_last_frame_index(uint64_t index) { m_lastFrameIndex = index; }

	glm::ivec2 get_min() const { return m_min; }
	glm::ivec2 get_max() const { return m_max; }

	glm::ivec2 get_center() const { return (m_min + m_max) / 2; }
	void destroy();
private:
	uint64_t m_lastFrameIndex;
	uint32_t m_id;
	glm::ivec2 m_min;
	glm::ivec2 m_max;
	uint32_t m_lodLevel;

	Mesh* m_mesh;
	bool m_loaded = false;
};