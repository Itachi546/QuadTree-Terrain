#pragma once

#include <stdint.h>
#include <vector>

class Framebuffer;
class Pipeline;
class RenderPass;
class Entity;
class Context;
class ShaderBindings;

class OutlineFX
{
public:
	void init(uint32_t width, uint32_t height);
	void resize(uint32_t width, uint32_t height);
	void render(Context* context, std::vector<Entity*> entities, ShaderBindings* globalBindings);

	ShaderBindings* get_output_image_bindings() { return m_outlineOutputBindings; }
	void destroy();
private:
	Pipeline* m_prepassPipeline;
	Pipeline* m_outlinePipeline;

	RenderPass* m_renderpass;

	Framebuffer* m_prepassAttachment;
	Framebuffer* m_outlineAttachment;

	ShaderBindings* m_bindings;
	ShaderBindings* m_outlineOutputBindings;

	
	uint32_t m_width;
	uint32_t m_height;
};