#pragma once

#include "core/base.h"
#include "core/math.h"
#include <vector>
#include <stdint.h>

class Pipeline;
class Mesh;
class Context;
class ShaderBindings;
class QuadTree;
class Camera;
class TerrainChunkManager;
class TerrainStream;

class Terrain
{

public:
	Terrain(Context* context, Ref<TerrainStream> stream);

	float get_height(glm::vec3 position);
	void update(Context* context, Ref<Camera> camera);
	void update(glm::vec3 position);
	void render(Context* context, ShaderBindings** uniformBindings, int count);
	void destroy();
private:
	Pipeline* m_pipeline;
	Ref<TerrainChunkManager> m_chunkManager;
	Ref<TerrainStream> m_stream;
	uint32_t minchunkSize = 64;
	uint32_t m_maxLod;

	Ref<QuadTree> m_quadTree;
};