#pragma once

#include <stdint.h>
#include "graphics_enums.h"

class RenderPass;
class Pipeline;
class VertexBuffer;
class IndexBuffer;
class UniformBuffer;
class ShaderBindings;
class Texture;
class Framebuffer;
class Context
{
public:
	virtual void begin() = 0;
	virtual void begin_renderpass(RenderPass* renderPass, Framebuffer* framebuffer) = 0;
	virtual void end_renderpass() = 0;
	virtual void set_graphics_pipeline(Pipeline* pipeline) = 0;
	virtual void end() = 0;

	virtual void copy(VertexBuffer* buffer, void* data, uint32_t offsetInByte, uint32_t sizeInByte) = 0;
	virtual void copy(IndexBuffer* buffer, void* data, uint32_t offsetInByte, uint32_t sizeInByte) = 0;
	virtual void copy(UniformBuffer* buffer, void* data, uint32_t offsetInByte, uint32_t sizeInByte) = 0;

	virtual void draw(uint32_t vertexCount) = 0;
	virtual void draw_indexed(uint32_t indexCount) = 0;

	virtual void set_buffer(VertexBuffer* buffer, uint32_t offset) = 0;
	virtual void set_buffer(IndexBuffer* buffer, uint32_t offset) = 0;

	virtual void transition_layout_for_shader_read(Texture* texture, bool depthTexture) = 0;

	virtual void set_shader_bindings(ShaderBindings** shaderBindings, uint32_t count) = 0;
	virtual void set_uniform(ShaderStage shaderStage, uint32_t offset, uint32_t size, void* data) = 0;
	virtual void set_line_width(float width) = 0;
	// Always call before set RenderPass
	void set_clear_color(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f)
	{
		m_clearValues.r = r;
		m_clearValues.g = g;
		m_clearValues.b = b;
		m_clearValues.a = a;
	}

	// Always call before set RenderPass
	void set_clear_depth(float depth = 1.0f)
	{
		m_clearValues.depth = depth;
	}

	uint32_t get_total_draw_calls()
	{
		return m_totalDrawCalls;
	}

	virtual RenderPass* get_global_renderpass() = 0;

	virtual ~Context(){}

protected:
	struct ClearValues
	{
		float r = 0.0f;
		float g = 0.0f;
		float b = 0.0f;
		float a = 1.0f;
		float depth = 1.0f;
	} m_clearValues;

	uint32_t m_totalDrawCalls = 0;
};