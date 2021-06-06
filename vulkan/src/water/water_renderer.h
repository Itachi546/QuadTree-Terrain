#pragma once

#include <stdint.h>
#include "core/math.h"

class Context;
class Pipeline;
class Texture;
class VertexBuffer;
class IndexBuffer;
class ShaderBindings;

class WaterRenderer
{
public:
	WaterRenderer(Context* context, Texture* displacementTexture, Texture* normalMapTexture);
	void render(Context* context, ShaderBindings** uniformBindings, glm::vec3 translate, uint32_t count);
	void destroy();
private:
	VertexBuffer* m_vbo;
	IndexBuffer* m_ibo;
	Pipeline* m_pipeline;
	ShaderBindings* m_bindings;

	unsigned int m_indicesCount;
};