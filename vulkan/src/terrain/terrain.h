#pragma once

#include "core/base.h"
#include "perlin_noise.h"
#include <stdint.h>

class TerrainStream;
class Pipeline;
class Mesh;
class Context;
class ShaderBindings;

class Terrain
{

public:
	Terrain(Context* context, Ref<TerrainStream> stream);

	void render(Context* context, ShaderBindings* uniformBindings);

	void destroy();
	float get_height(int x, int y);
private:
	siv::PerlinNoise* noise;
	Ref<TerrainStream> m_stream;
	Ref<Mesh> m_mesh;
	Pipeline* m_pipeline;
};