#pragma once

#include "core/base.h"
#include <vector>

class Context;
class Pipeline;
class Entity;
class ShaderBindings;
class Texture;
class TerrainChunk;
class IndexBuffer;

class Grass
{
public:
	Grass(Context* context);
	void render(Context* context, Entity* entity, ShaderBindings** bindings, uint32_t count, float elapsedTime);
	void render(Context* context, std::vector<TerrainChunk*> chunks, IndexBuffer* ib, uint32_t indicesCount, ShaderBindings** bindings, uint32_t count, float elapsedTime);
	void destroy();
private:
	Pipeline* m_pipeline;
	Texture* m_distortionTexture;
	Texture* m_noiseTexture;
	ShaderBindings* m_bindings;
};