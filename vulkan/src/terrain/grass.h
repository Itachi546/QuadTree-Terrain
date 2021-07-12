#pragma once

#include "core/base.h"

class Context;
class Pipeline;
class Entity;
class ShaderBindings;
class Texture;

class Grass
{
public:
	Grass(Context* context);
	void render(Context* context, Entity* entity, ShaderBindings** bindings, uint32_t count, float elapsedTime);
	void destroy();
private:
	Pipeline* m_pipeline;
	Texture* m_distortionTexture;
	Texture* m_noiseTexture;
	ShaderBindings* m_bindings;
};