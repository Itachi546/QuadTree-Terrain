#pragma once

#include "core/math.h"

class ShaderBindings;
class Context;
class Pipeline;
class Texture;

class NormalMapGenerator
{
public:
	NormalMapGenerator(Context* context, Texture* displacementMap);
	void generate(Context* context);
	Texture* get_normal_texture() { return m_normalTexture; }
	void destroy();
private:
	Texture* m_normalTexture;
	Pipeline* m_pipeline;
	ShaderBindings* m_bindings;
	
	glm::vec2 m_invResolution;
	int m_N;
};