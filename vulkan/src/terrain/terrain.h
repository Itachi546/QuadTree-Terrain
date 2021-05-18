#pragma once

#include "core/base.h"
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

	void render(Context* context, ShaderBindings** uniformBindings, int count);

	Ref<Mesh> get_mesh() { return m_mesh; }

	void destroy();
	// between x (0, 1), y(0, 1)
	float get_height(int x, int y);
private:
	Ref<TerrainStream> m_stream;
	Ref<Mesh> m_mesh;
	Pipeline* m_pipeline;
};