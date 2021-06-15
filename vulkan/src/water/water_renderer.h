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
	WaterRenderer(Context* context);
	void render(Context* context, ShaderBindings** uniformBindings, glm::vec3 cameraPos, glm::vec3 translate, uint32_t count);
	void destroy();
private:
	VertexBuffer* m_vbo;
	IndexBuffer* m_ibo;
	Pipeline* m_pipeline;
	unsigned int m_indicesCount;
};