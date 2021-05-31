#pragma once

#include "core/base.h"
#include "core/math.h"
#include "core/ray.h"
#include <vector>
#include <stdint.h>

class Pipeline;
class Mesh;
class Context;
class ShaderBindings;

class QuadTree;
class Camera;
class TerrainChunk;
class TerrainStream;

class Terrain
{

public:
	Terrain(Context* context, Ref<TerrainStream> stream);

	// Binary search method
	// @TODO works for now but need improvement
	// doesn't work properly in steep slope
	bool ray_cast(const Ray& ray, glm::vec3& p_out);

	float get_height(glm::vec3 position);
	void update(Context* context, Ref<Camera> camera);

	void render(Context* context, Ref<Camera> camera, ShaderBindings** uniformBindings, int count, bool depthPass = false);
	void destroy();
private:
	Pipeline* m_pipeline;
	Pipeline* m_wireframePipeline;
	Pipeline* m_activePipeline;

	Ref<TerrainStream> m_stream;
	Ref<QuadTree> m_quadTree;

	uint32_t m_minchunkSize = 64;
	uint32_t m_maxLod;
	int m_influenceRadius = 10;
	
	const float m_maxRayCastDistance = 500.0f;
	const int m_maxHeight = 150;

	glm::vec4 m_terrainIntersection = glm::vec4(0.0f);
	bool binary_search(const glm::vec3& p0, const glm::vec3& p1, float minThreshold, glm::vec3& p_out);

	void operation_average();
};