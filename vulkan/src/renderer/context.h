#pragma once

#include <stdint.h>
#include "graphics_enums.h"

class RenderPass;
class Pipeline;
class VertexBuffer;
class IndexBuffer;
class UniformBuffer;
class ShaderStorageBuffer;
class IndirectBuffer;

class ShaderBindings;
class Texture;
class Framebuffer;
class GraphicsWindow;
class GpuTimestampQuery;

class Context
{
public:
	virtual GraphicsWindow* get_window() = 0;

	virtual void acquire_swapchain_image() = 0;
	//Start of commandBuffer
	virtual void begin() = 0;
	virtual void begin_renderpass(RenderPass* renderPass, Framebuffer* framebuffer) = 0;
	virtual void end_renderpass() = 0;
	virtual void set_pipeline(Pipeline* pipeline) = 0;
	//End of commandBuffer
	virtual void begin_compute() = 0;
	virtual void end_compute() = 0;


	virtual void end() = 0;
	virtual void present() = 0;

	virtual void copy(VertexBuffer* buffer, void* data, uint32_t offsetInByte, uint32_t sizeInByte) = 0;
	virtual void copy(IndexBuffer* buffer, void* data, uint32_t offsetInByte, uint32_t sizeInByte) = 0;
	virtual void copy(UniformBuffer* buffer, void* data, uint32_t offsetInByte, uint32_t sizeInByte) = 0;
	virtual void copy(ShaderStorageBuffer* buffer, void* data, uint32_t offsetInByte, uint32_t sizeInByte) = 0;
	virtual void copy(IndirectBuffer* buffer, void* data, uint32_t offsetInByte, uint32_t sizeInByte) = 0;

	virtual void copy(Texture* texture, void* data, uint32_t sizeInByte) = 0;

	virtual void draw(uint32_t vertexCount) = 0;
	virtual void draw_indexed(uint32_t indexCount) = 0;
	virtual void draw_indexed_indirect(IndirectBuffer* buffer, uint32_t offset, uint32_t drawCount, uint32_t stride) = 0;

	virtual void dispatch_compute(uint32_t workGroupSizeX, uint32_t workGroupSizeY, uint32_t workGroupSizeZ) = 0;

	virtual void set_buffer(VertexBuffer* buffer, uint32_t offset) = 0;
	virtual void set_buffer(IndexBuffer* buffer, uint32_t offset) = 0;



	virtual void transition_layout_for_shader_read(Texture** texture, uint32_t count) = 0;
	virtual void transition_layout_for_compute_read(Texture** texture, uint32_t count) = 0;

	virtual void update_pipeline(Pipeline* pipeline, ShaderBindings** shaderBindings, uint32_t count) = 0;
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

	virtual void reset_query(GpuTimestampQuery* query) = 0;
	//virtual void end_query(GpuTimestampQuery* query) = 0;
	virtual void write_timestamp(GpuTimestampQuery* query, uint32_t queryIndex) = 0;
	virtual void get_result(GpuTimestampQuery* query, uint32_t firstQuery, uint32_t queryCount, void* output) = 0;

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
};